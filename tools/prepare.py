"""Adapt the pinned upstream frontend, preserving its semantic game actions.

The upstream checkout is deliberately unmodified. Device code and all visible
layouts are supplied by src/*.inc; generated C remains inspectable in build/.
"""
from pathlib import Path
import re
if __package__:
    from .resource_paths import header
else:
    from resource_paths import header

ROOT = Path(__file__).resolve().parents[1]
UP = ROOT / 'upstream'

def function_span(source, name):
    match = re.search(r'(?m)^(?:static )?[^\n;{}]+\b' + re.escape(name) + r'\([^;]*?\)\s*\{', source)
    if not match:
        raise ValueError('Upstream function missing: ' + name)
    start = source.index('{', match.start())
    depth = 0
    tokens = re.finditer(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*[\s\S]*?\*/|//[^\n]*|[{}]', source[start:])
    for token in tokens:
        if token.group() == '{': depth += 1
        elif token.group() == '}':
            depth -= 1
            if depth == 0:
                return match.start(), start + token.end()
    raise ValueError('Unbalanced function: ' + name)

def replace_function(source, name, replacement=''):
    start, end = function_span(source, name)
    return source[:start] + replacement + source[end:]

def extract(source, name):
    start, end = function_span(source, name)
    return source[start:end] + '\n'

def prepare(out):
    out.mkdir(parents=True, exist_ok=True)
    (out / 'duel_paths.h').write_text(header(), encoding='ascii')
    source = (UP / 'src/duel_h1.c').read_text(encoding='utf8')
    source = source.replace('#include "h1_sdk.h"', '#include "platform9588.h"')
    source = source.replace('screen[480*272]', 'screen[240*320]')
    source = source.replace('#include "duel_assets.inc"', '#include "assets9588.inc"')
    source = source.replace('#include "duel_scene.inc"', '#include "scene9588.inc"')
    for name in ('key_held', 'pen', 'poll_keyboard', 'h1_app_main', 'details_draw', 'result_draw', 'records_draw', 'editor_draw', 'board_draw', 'opening_draw'):
        source = replace_function(source, name)
    source = replace_function(source, 'redraw', '#include "ui9588.inc"')
    # Legacy touch is used only by upstream activate() to dispatch title actions.
    # Physical 9588 touches use portrait_touch(), whose hitboxes match the new UI.
    source = re.sub(r'\btouch\(', 'legacy_touch(', source)
    source = replace_function(source, 'hand_visible', 'static unsigned hand_visible(void){unsigned n=joey_duel_state.players[0].hand_count;return n>6?6:n;}')
    source = source.replace('hand_scroll+10', 'hand_scroll+6').replace('hand_scroll=focus-9', 'hand_scroll=focus-5')
    source = replace_function(source, 'editor_page_size', 'static unsigned editor_page_size(unsigned pane){(void)pane;return 5;}')
    source += '\n#include "entry9588.inc"\n'
    (out / 'duel9588.c').write_text(source, encoding='utf8')
    assets = (UP / 'data/duel_assets.inc').read_text(encoding='utf8')
    assets = assets[:assets.index('static int native_load(')] + extract(assets, 'native_load') + extract(assets, 'native_string') + extract(assets, 'native_texture')
    assets = assets.replace('static int native_texture(unsigned card,unsigned kind,uint16_t **palette,uint8_t **pixels){','static int native_texture(unsigned card,unsigned kind,uint16_t **palette,uint8_t **pixels){joey_audio_tick();')
    (out / 'assets9588.inc').write_text(assets + '\n#include "graphics9588.inc"\n', encoding='utf8')
    ai = (UP / 'src/duel_ai_scene.inc').read_text(encoding='utf8')
    (out / 'duel_ai_scene.inc').write_text(replace_function(ai, 'ai_draw'), encoding='utf8')
    font = (UP / 'data/duel_font.inc').read_text(encoding='utf8')
    font = re.sub(r'\bu16\(', 'font_read16(', font)
    font = re.sub(r'\bu32\(', 'font_read32(', font)
    (out / 'duel_font.inc').write_text(font, encoding='utf8')

if __name__ == '__main__':
    prepare(ROOT / 'build')
