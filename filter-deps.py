import re

def filter_dot_file(input_file, output_file):
    # Read input file
    with open(input_file, 'r') as f:
        content = f.read()
    
    # Split into lines while preserving header
    lines = content.split("}")
    header = lines[0] + "}"
    rest = "".join(lines[1:])
    
    # Split the rest into node definitions and edges
    node_lines = [line for line in rest.splitlines() if 'label = "' in line]
    edge_lines = [line for line in rest.splitlines() if '->' in line]
    
    # Find nodes to keep (not starting with pico_ or hardware_)
    nodes_to_keep = set()
    for line in node_lines:
        if 'label = "' in line:
            node_id = re.search(r'\"(node\d+)\"', line).group(1)
            label = re.search(r'label\s*=\s*\"([^"]+)\"', line).group(1)
            if not (label.startswith('pico_') or label.startswith('hardware_') or label.startswith('cyw43_') or label.startswith('tinyusb_') or label.startswith('blockdevice_') or label.startswith('filesystem_')):
                nodes_to_keep.add(node_id)
    
    # Filter node definitions
    filtered_nodes = []
    for line in node_lines:
        node_id = re.search(r'\"(node\d+)\"', line).group(1)
        if node_id in nodes_to_keep:
            filtered_nodes.append(line)
    
    # Filter edges to only include kept nodes
    filtered_edges = []
    for line in edge_lines:
        if '->' in line:
            source, target = re.findall(r'\"(node\d+)\"', line)
            if source in nodes_to_keep and target in nodes_to_keep:
                filtered_edges.append(line)
    
    # Write output
    with open(output_file, 'w') as f:
        f.write(header + "\n")
        f.write("\n".join(filtered_nodes))
        f.write("\n")
        f.write("\n".join(filtered_edges))
        f.write("\n}")

if __name__ == '__main__':
    filter_dot_file('filtered_deps.dot', 'filtered_deps2.dot')