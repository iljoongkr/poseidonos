#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import json


def check_result(detail):
    isOnline = json_parser.is_online(detail)
    if isOnline == False:
        return "pass"
    return "fail"

def set_result():
    out = cli.get_pos_info()
    code = json_parser.get_response_code(out)
    result = check_result(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    pos.mbr_reset()
    pos.start_pos()

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()