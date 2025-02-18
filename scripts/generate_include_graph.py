import os
import re
from pathlib import Path

def find_includes(file_path):
    includes = []
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        # Match both <header.h> and "header.h" includes
        matches = re.findall(r'#include [<"](.*?)[>"]', content)
        includes.extend(matches)
    return includes

def generate_dot(root_dir):
    dot_content = ['digraph G {', '  rankdir=LR;', '  node [shape=box];']
    
    # Track processed files to avoid duplicates
    processed = set()
    
    # Find all source files
    for ext in ['.cpp', '.c', '.h', '.hpp']:
        for file_path in Path(root_dir).rglob(f'*{ext}'):
            if 'build' in file_path.parts:
                continue
                
            source_name = str(file_path.relative_to(root_dir))
            source_name = source_name.replace('\\', '/')
            
            if source_name in processed:
                continue
                
            processed.add(source_name)
            includes = find_includes(file_path)
            
            for include in includes:
                # Clean up include path
                include = include.replace('\\', '/')
                dot_content.append(f'  "{source_name}" -> "{include}";')
    
    dot_content.append('}')
    return '\n'.join(dot_content)

if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dot_content = generate_dot(root_dir)
    
    # Write DOT file
    with open('include_graph.dot', 'w') as f:
        f.write(dot_content)