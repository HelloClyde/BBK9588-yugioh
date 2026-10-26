"""Verify the new Chinese resource/save directory through normal firmware UI."""
import argparse
import hashlib
import json
import time
from pathlib import Path
import live
import verify_firmware as v

ROOT = Path(__file__).resolve().parents[1]


def wait(predicate, label, timeout=15):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        if predicate():
            return
        time.sleep(.15)
    raise AssertionError((label, v.diag()))


def touch(x, y, predicate, label):
    live.touch(x, y)
    wait(predicate, label)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--base', required=True)
    ap.add_argument('--program', default=live.PROGRAM_PATH)
    ap.add_argument('--save-cycles', type=int, default=4)
    args = ap.parse_args()
    live.BASE = args.base
    live.PROGRAM_PATH = args.program
    build = json.loads((ROOT / 'dist/BUILD.json').read_text(encoding='utf8'))
    assert build['resource_directory'] == '应用/数据/游戏/YUGIOH'
    wait(lambda: v.diag()['view'] == 2 and v.u32('scene') == 0
         and not v.u32('about_open'), 'start at the running game title menu')
    report = dict(passed=False, bda_sha256=build['bda_sha256'],
                  sdk_commit=build['sdk_commit'], upstream_commit=build['upstream_commit'],
                  text_sha256=build['text_sha256'], load_image_sha256=build['load_image_sha256'],
                  resource_directory=build['resource_directory'], resource_roots=build['resource_roots'],
                  guest_memory_written=False, physical_9588_tested=False,
                  firmware_tested_drives=['A'], b_drive_host_tests='docs/verification/resource-paths.json',
                  scope='resource loading, menu input, dual-slot save and exit/relaunch', checks={})
    report['program_at_start'] = v.verify_program()
    assert v.u32('native_resource_drive') == 0
    observed = {}
    for name, expected in build['required_external_resources'].items():
        data = live.req('/api/files/export', path=live.RESOURCE_PATH + '/' + name).content
        assert len(data) == expected['bytes'] and hashlib.sha256(data).hexdigest() == expected['sha256']
        observed[name] = expected['sha256']
    report['checks']['five_resources_at_new_path'] = observed
    for _ in range(6):
        if v.u32('native_menu_focus') == 0:break
        live.key(4)
    live.key(5);wait(lambda: v.u32('native_menu_focus') == 1, 'menu down')
    live.key(4);wait(lambda: v.u32('native_menu_focus') == 0, 'menu up')
    live.req('/api/key', True, code=5, down=1)
    try:
        time.sleep(.8)
    finally:
        live.req('/api/key', True, code=5, down=0)
    wait(lambda: v.u32('native_menu_focus') == 1, 'held menu key')
    live.key(4);wait(lambda: v.u32('native_menu_focus') == 0, 'restore menu focus')
    report['checks']['short_and_held_menu_keys'] = {'passed': True}
    touch(120, 180, lambda: v.u32('scene') == 3, 'open deck')
    before = v.diag()['profile']; base = before[10]
    assert v.diag()['profile'][7] == 0
    remove = base > 40
    count = base - 1 if remove else base + 1
    saves = []
    def save_and_reopen(sequence):
        # Allow the touch release and its firmware ESC synthesis to settle.
        # Commit using the ordinary keyboard save-and-leave confirmation.
        time.sleep(.7)
        if not v.u32('editor_leaving'):live.key(9)
        wait(lambda: v.u32('editor_leaving') == 1, 'save-and-leave prompt')
        live.key(10)
        saved = v.wait_saved(sequence)
        wait(lambda: v.u32('scene') == 0, 'leave after save')
        result = v.save_export(saved[1])
        v.verify_program()
        live.key(10)
        wait(lambda: v.u32('scene') == 3, 'reopen saved deck')
        return saved, result
    for cycle in range(args.save_cycles):
        touch(145 if remove else 90, 305, lambda: v.diag()['profile'][10] == count, 'edit deck')
        saved1, slot1 = save_and_reopen(before[0] + cycle * 2 + 1)
        assert slot1['main_count'] == count
        touch(90 if remove else 145, 305, lambda: v.diag()['profile'][10] == base, 'restore deck')
        saved2, slot2 = save_and_reopen(before[0] + cycle * 2 + 2)
        assert slot2['main_count'] == base
        assert saved1[1] != saved2[1]
        saves.extend([slot1, slot2])
        print('Verified save cycle', cycle + 1, flush=True)
    report['checks']['dual_slot_save_in_new_directory'] = {'passed': True, 'saves': saves, 'program_checked_after_each_save': True}
    live.key(9);wait(lambda: v.u32('scene') == 0, 'return to menu')
    live.shot('alpha2-menu-new-path')
    live.key(9)
    wait(lambda: v.u32('exit_requested') == 1, 'normal app exit')
    time.sleep(2)
    tick = v.memory('joey_native_diagnostic')[12:16]
    time.sleep(.5)
    assert v.memory('joey_native_diagnostic')[12:16] == tick, 'App loop did not exit'
    live.shot('alpha2-desktop-after-exit')
    live.key(10)
    wait(lambda: v.u32('exit_requested') == 0 and v.u32('scene') == 0
         and v.diag()['profile'][0] == slot2['sequence'], 'relaunch saved profile')
    tick = v.memory('joey_native_diagnostic')[12:16]
    time.sleep(.5)
    assert v.memory('joey_native_diagnostic')[12:16] != tick
    assert v.u32('native_resource_drive') == 0
    touch(120, 180, lambda: v.u32('scene') == 3, 'reopen deck')
    assert v.diag()['profile'][10] == base
    assert v.save_export(saved2[1]) == slot2
    live.key(9);wait(lambda: v.u32('scene') == 0, 'return after relaunch')
    report['checks']['normal_exit_relaunch_saved_profile'] = {'passed': True, 'save': slot2}
    report['program_after_relaunch'] = v.verify_program()
    report['passed'] = True
    (ROOT / 'build/alpha2-paths-firmware.json').write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
