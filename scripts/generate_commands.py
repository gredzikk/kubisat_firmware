import json
import os
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

def generate_source(json_data):
    timestamp = int(time.time())
    
    source = f"""// Auto-generated file - DO NOT EDIT
#include "groups.h"

const uint32_t COMMANDS_FILE_VERSION = {timestamp}u;

std::vector<Group> getGroups()
{{
    return {{"""
    
    for group in json_data["groups"]:
        source += f"""
        {{
            {group["id"]},
            "{group["name"]}",
            {{"""
        for cmd in group["commands"]:
            source += f"""
                {{ {cmd["id"]}, "{cmd["name"]}", CommandAccessLevel::{cmd["access"]}, ValueUnit::{cmd["unit"]} }},"""
        source += """
            }
        },"""
    
    source += """
    };
}"""
    return source

def main():
    json_path = os.path.join(SCRIPT_DIR, "commands.json")
    source_path = os.path.join(PROJECT_ROOT, "frame_model", "groups.cpp")
    
    print(f"Reading commands from: {json_path}")
    print(f"Generating output to: {source_path}")
    
    with open(json_path, "r") as f:
        data = json.load(f)
    
    with open(source_path, "w") as f:
        f.write(generate_source(data))

if __name__ == "__main__":
    main()