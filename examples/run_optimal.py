import os
import argparse

parser = argparse.ArgumentParser(description='Apply constructive_linear algorithm to benchmark trace')
parser.add_argument('benchname', help='Target benchmark trace used to get page difference')

args = parser.parse_args()

trace_dir='/home/yangxr/downloads/test_trace/hot_dist_5_15/' + args.benchname + '/'
output_dir="/home/yangxr/downloads/test_trace/res/" + args.benchname + "/PGM_Index_v1/VPN"

global_file_time = 0

def init_local_env(filename):
    global global_file_time

    base_filename = os.path.basename(filename)
    global_file_time = int(base_filename.split('.')[0].split('_')[1])
    print(f"processing bench: {args.benchname} time: {global_file_time}")

if __name__ == '__main__':
    for filename in os.listdir(trace_dir):
        if (filename.endswith('hot_v_5_15.out')):
            init_local_env(filename)
            os.system(f"./optimal_plr {trace_dir}/{filename} {output_dir}/{args.benchname}_{global_file_time}s.pgm_index_v1.segs.csv")