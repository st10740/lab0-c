import subprocess
import matplotlib.pyplot as plt

def modify_test_sort(filename, target_macro, new_macro):
    with open(filename, 'r') as file:
        lines = file.readlines()

    modified_lines = []
    for line in lines:
        if target_macro in line:
            line = line.replace(target_macro, str(new_macro))
        modified_lines.append(line)

    with open(filename, 'w') as file:
        file.writelines(modified_lines)


def write_cmd(trace_file, num):
    command = f'new\nih RAND {num}\nsort'
    with open(trace_file, 'w+') as file:
        file.write(command)


def run_perf(trace_file):
    perf_command = ['perf', 'stat', '--repeat', '5', '-e', 'cycles,cache-misses', './qtest', '-f', trace_file]
    process = subprocess.Popen(perf_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    
    perf_output = (out + err).decode('utf-8')
    cpu_times = 0
    cache_misses = 0
    
    lines = perf_output.split('\n')
    for line in lines:
        if 'cycles' in line:
            cpu_times = int(line.split()[0].replace(',', ''))
        elif 'cache-misses' in line:
            cache_misses = int(line.split()[0].replace(',', ''))
    
    return cpu_times, cache_misses


def run_tests(): 
    queue_c_file = 'queue.c'
    merge_sort_macro = '#define lsort 0\n'
    list_sort_macro = '#define lsort 1\n'
    
    node_nums = list(range(1000, 500000, 1000))
    merge_sort_cycles = []
    list_sort_cycles = []
    merge_sort_cache_misses = []
    list_sort_cache_misses = []
    
    # run tests
    for sort in ['merge_sort', 'list_sort']:
        print(sort)

        if sort == 'merge_sort':
            modify_test_sort('queue.c', list_sort_macro, merge_sort_macro)
        else:
            modify_test_sort('queue.c', merge_sort_macro, list_sort_macro)
        
        subprocess.run(['make'])
        trace_file = 'traces/trace-sort.cmd'
        for size in node_nums:
            print(f'node_nums: {size}')
            write_cmd(trace_file, size)
            cycles, cache_misses = run_perf(trace_file)
            if sort == 'merge_sort':
                merge_sort_cycles.append(cycles)
                merge_sort_cache_misses.append(cache_misses)
            else:
                list_sort_cycles.append(cycles)
                list_sort_cache_misses.append(cache_misses)

    # plot tests results
    # CPU cycles
    plt.plot(node_nums, merge_sort_cycles, label='Merge sort')
    plt.plot(node_nums, list_sort_cycles, label='list_sort')
    plt.title('CPU cycles')
    plt.xlabel('number of nodes')
    plt.ylabel('CPU cycles')
    plt.legend()
    plt.show()
    # cache misses
    plt.plot(node_nums, merge_sort_cache_misses, label='Merge sort')
    plt.plot(node_nums, list_sort_cache_misses, label='list_sort')
    plt.title('cache misses')
    plt.xlabel('number of nodes')
    plt.ylabel('cache misses')
    plt.legend()
    plt.show()


if __name__ == "__main__":
    run_tests()