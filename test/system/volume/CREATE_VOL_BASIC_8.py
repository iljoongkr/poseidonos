#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import test_result
import pos_constant
import MOUNT_ARRAY_BASIC
import volume

VOL_NAME_PREFIX = "vol"
VOL_SIZE = pos_constant.SIZE_1MB * 100
VOL_IOPS = 10
VOL_BW = 10
vol_list = []

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    expected_list = []
    for i in range(0, pos_constant.MAX_VOL_CNT):
        expected_list.append(volume.Volume(VOL_NAME_PREFIX + str(i+1), VOL_SIZE, VOL_IOPS, VOL_BW))
    
    data = json.loads(detail)
    actual_list = []
    for item in data['Response']['result']['data']['volumes']:
        vol = volume.Volume(item['name'], item['total'], item['maxiops'], item['maxbw'])
        actual_list.append(vol)

    if len(actual_list) != len(expected_list):
        return "fail"
    
    for actual in actual_list:
        checked = False
        for expected in expected_list:
            if actual.name == expected.name and actual.total == expected.total and actual.maxiops == expected.maxiops and actual.maxbw == expected.maxbw:
                checked = True
                break
        if checked == False:
            return "fail"
    return "pass"

def set_result(detail):
    out = cli.list_volume("")
    result = check_result(out)
    code = json_parser.get_response_code(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_BASIC.execute()
    for i in range(0, pos_constant.MAX_VOL_CNT):
        print ("creating " + VOL_NAME_PREFIX + str(i+1))
        out = cli.create_volume(VOL_NAME_PREFIX + str(i+1), str(VOL_SIZE), str(VOL_IOPS), str(VOL_BW), "")
        ret = json_parser.get_response_code(out)
        if ret != 0:
            break;
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    pos.kill_pos()