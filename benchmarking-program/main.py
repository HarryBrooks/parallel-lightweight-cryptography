import os
import sys
import argparse
import platform
import subprocess
import hashlib
import requests
import json
import math

from statistics import mean
from tqdm import tqdm
from time import time
from ctypes import CDLL, Structure, c_int, c_double, c_long

iterations = 15

class Benchmark(Structure):
    _fields_ = [
        ("ret", c_int),
        ("time_encrypting", c_double),
        ("time_decrypting", c_double)
    ]

class BenchmarkSmall(Structure):
    _fields_ = [
        ("ret", c_int),
        ("time_encrypting", c_double),
        ("time_decrypting", c_double),
        ("min_throughput_encrypting", c_double),
        ("max_throughput_encrypting", c_double),
        ("average_throughput_encrypting", c_double),
        ("min_throughput_decrypting", c_double),
        ("max_throughput_decrypting", c_double),
        ("average_throughput_decrypting", c_double),
        ("encryption_times", c_double * 16),
        ("decryption_times", c_double * 16)
    ]

class BenchmarkMemory(Structure):
    _fields_ = [
        ("start_vmrss", c_long * 16),
        ("start_vmsize", c_long * 16),
        ("end_vmrss", c_long * 16),
        ("end_vmsize", c_long * 16)
    ]

def benchmark(cBenchmark):
    cBenchmark.benchmark.restype = Benchmark
    encryption_times = []
    decryption_times = []
    for _ in tqdm(range(iterations)):
        c_return = cBenchmark.benchmark()
        encryption_times.append(c_return.time_encrypting)
        decryption_times.append(c_return.time_decrypting)
    return(str(mean(encryption_times)),str(mean(decryption_times)))

def benchmark_small(cBenchmark, pi):
    cBenchmark.benchmark.restype = BenchmarkSmall
    min_throughput_encrypting = 0
    max_throughput_encrypting = 0
    average_throughput_encrypting = 0
    min_throughput_decrypting = 0
    max_throughput_decrypting = 0
    average_throughput_decrypting = 0
    encrypting_times = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    decrypting_times = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    for _ in tqdm(range(iterations)):
        c_return = cBenchmark.benchmark()
        min_throughput_encrypting += c_return.min_throughput_encrypting
        max_throughput_encrypting += c_return.max_throughput_encrypting
        average_throughput_encrypting += c_return.average_throughput_encrypting
        min_throughput_decrypting += c_return.min_throughput_decrypting
        max_throughput_decrypting += c_return.max_throughput_decrypting
        average_throughput_decrypting += c_return.average_throughput_decrypting
        for i in range(12):
            encrypting_times[i] += c_return.encryption_times[i]
            decrypting_times[i] += c_return.decryption_times[i]
    for i in range(12):
        encrypting_times[i] = max(encrypting_times[i] / iterations, math.pow(10,-100))
        decrypting_times[i] = max(decrypting_times[i] / iterations, math.pow(10,-100))

    if pi:
        encrypting_times = encrypting_times[:9]
        decrypting_times = decrypting_times[:9]
    return (min_throughput_encrypting/iterations, max_throughput_encrypting/iterations,
            average_throughput_encrypting/iterations, min_throughput_decrypting/iterations,  
            max_throughput_decrypting/iterations, average_throughput_decrypting/iterations,
            encrypting_times, decrypting_times)

def benchmark_latency(cBenchmark):
    cBenchmark.benchmark.restype = Benchmark
    encrypting_times = []
    decrypting_times = []
    for _ in tqdm(range(iterations)):
        c_return = cBenchmark.benchmark()
        encrypting_times.append(c_return.time_encrypting)
        decrypting_times.append(c_return.time_decrypting) 
    return max(mean(encrypting_times),math.pow(10,-100)), max(mean(decrypting_times),math.pow(10,-100))

def benchmark_memory(cBenchmark):
    cBenchmark.benchmark.restype = BenchmarkMemory
    start_vmrss = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    start_vmsize = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    end_vmrss = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    end_vmsize = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    for _ in tqdm(range(iterations)):
        c_return = cBenchmark.benchmark()
        for i in range(16):
            start_vmrss[i] += c_return.start_vmrss[i]
            start_vmsize[i] += c_return.start_vmsize[i]
            end_vmrss[i] += c_return.end_vmrss[i]
            end_vmsize[i] += c_return.end_vmsize[i]
    for i in range(16):
        start_vmrss[i] = start_vmrss[i] / iterations
        start_vmsize[i] = start_vmsize[i] / iterations
        end_vmrss[i] = end_vmrss[i] / iterations
        end_vmsize[i] = end_vmsize[i] / iterations
    return start_vmrss, start_vmsize, end_vmrss, end_vmsize

def run_benchmark(algorithm_name, original, pi, hamilton):

    startArgs = ['cc', '-fPIC', '-shared', '-o']
    endArgsNoOpt = ['-Wall', '-Wextra', '-Wshadow', '-O0']
    endArgsSIMD = ['-Wall', '-Wextra', '-Wshadow', '-O2']
    endArgsMP = ['-Wall', '-Wextra', '-Wshadow', '-fopenmp', '-O2']
    endArgsMPMac = ['-Wall', '-Wextra', '-Wshadow', '-Xpreprocessor', '-fopenmp', '-lomp', '-O2']

    results = dict()

    executable_folder = algorithm_name + '_executables'

    os.system('mkdir ' + executable_folder)

    results['system'] = platform.system()
    results['machine'] = platform.machine()

    executable_size = 1
    executable_num = 1
    
    c_files = []
    for subfile in os.listdir(algorithm_name):
        if '.c' in subfile and 'benchmark' not in subfile:
            c_files.append(algorithm_name + os.sep + subfile)
    
    os.system('cp crypto_aead.h ' + algorithm_name + os.sep + 'crypto_aead.h')
    if platform.system() == 'Linux' and not pi and not hamilton:
        os.system('cp memory.h ' + algorithm_name + os.sep + 'memory.h')
        os.system('cp memory.c ' + algorithm_name + os.sep + 'memory.c')
        os.system('cp benchmark_memory.c ' + algorithm_name + os.sep + 'benchmark_memory.c')
        subprocess.run(['cc', '-fPIC', '-shared', '-pthread', '-o', executable_folder + os.sep + 'executable_mem.so', algorithm_name + os.sep + 'benchmark_memory.c', algorithm_name + os.sep + 'memory.c'] + c_files + endArgsSIMD)
        print("Benchmarking memory 1KB to 1MB")
        cBenchmark = CDLL(executable_folder + os.sep + 'executable_mem.so')
        results['memory'] = {}
        results['memory']['start_vmrss'], results['memory']['start_vmsize'], results['memory']['end_vmrss'], results['memory']['end_vmsize'] = benchmark_memory(cBenchmark)

        os.system('rm ' + algorithm_name + os.sep + 'memory.h')
        os.system('rm ' + algorithm_name + os.sep + 'memory.c')
        os.system('rm ' + algorithm_name + os.sep + 'benchmark_memory.c')
        os.system('rm ' + executable_folder + os.sep + 'executable_mem.so')

    # Run the standard NIST benchmarking to get a baseline for the time on which all algorithms should operate.

    print("Benchmarking standard NIST test vectors with optimisation")
    if not hamilton:
        os.system('cp benchmark_nist.c ' + algorithm_name + os.sep + 'benchmark_nist.c')
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable.so', algorithm_name + os.sep + 'benchmark_nist.c'] + c_files + endArgsSIMD)
    cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable.so')
    results['nist_O2'] = {}
    results['nist_O2']['encryption'], results['nist_O2']['decryption'] = benchmark(cBenchmark)
    executable_size += os.path.getsize(executable_folder + os.sep + 'executable.so')
    executable_num += 1

    print("Benchmarking standard NIST test vectors without optimisation")
    if not hamilton:
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable_1.so', algorithm_name + os.sep + 'benchmark_nist.c'] + c_files + endArgsNoOpt)
    cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_1.so')
    results['nist_O0'] = {}
    results['nist_O0']['encryption'], results['nist_O0']['decryption'] = benchmark(cBenchmark)
    executable_size += os.path.getsize(executable_folder + os.sep + 'executable_1.so')
    executable_num += 1
    
    if not original and not pi:
        print("Benchmarking standard NIST test vectors with optimisation and openmp")
        if not hamilton:
            if platform.system() == 'Darwin':
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_2.so', algorithm_name + os.sep + 'benchmark_nist.c'] + c_files + endArgsMPMac)
            else:
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_2.so', algorithm_name + os.sep + 'benchmark_nist.c'] + c_files + endArgsMP)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_2.so')
        results['nist_MP'] = {}
        results['nist_MP']['encryption'], results['nist_MP']['decryption'] = benchmark(cBenchmark)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_2.so')
        executable_num += 1
        if not hamilton:
            os.system('rm ' + executable_folder + os.sep + 'executable_2.so')
    if not hamilton:
        os.system('rm ' + algorithm_name + os.sep + 'benchmark_nist.c')
        os.system('rm ' + executable_folder + os.sep + 'executable.so')
        os.system('rm ' + executable_folder + os.sep + 'executable_1.so')

    if pi:
        os.system('cp benchmark_pi.c ' + algorithm_name + os.sep + 'benchmark_pi.c')

        print("Benchmarking 1 KB to 0.25 MB with optimisation")
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable_3.so', algorithm_name + os.sep + 'benchmark_pi.c'] + c_files + endArgsSIMD)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_3.so')
        results['small_O2'] = {}
        results['small_O2']['min_throughput_encrypting'], results['small_O2']['max_throughput_encrypting'], results['small_O2']['average_throughput_encrypting'], results['small_O2']['min_throughput_decrypting'], results['small_O2']['max_throughput_decrypting'], results['small_O2']['average_throughput_decrypting'], results['small_O2']['encrypting_times'], results['small_O2']['decrypting_times'] = benchmark_small(cBenchmark, pi)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_3.so')
        executable_num += 1
        
        print("Benchmarking 1 KB to 0.25 MB without optimisation")
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable_4.so', algorithm_name + os.sep + 'benchmark_pi.c'] + c_files + endArgsNoOpt)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_4.so')
        results['small_O0'] = {}
        results['small_O0']['min_throughput_encrypting'], results['small_O0']['max_throughput_encrypting'], results['small_O0']['average_throughput_encrypting'], results['small_O0']['min_throughput_decrypting'], results['small_O0']['max_throughput_decrypting'], results['small_O0']['average_throughput_decrypting'], results['small_O0']['encrypting_times'], results['small_O0']['decrypting_times'] = benchmark_small(cBenchmark, pi)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_4.so')
        executable_num += 1
        
        if not original:
            print("Benchmarking 1 KB to 0.25 MB with optimisation and openmp")
            if platform.system() == 'Darwin':
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_5.so', algorithm_name + os.sep + 'benchmark_pi.c'] + c_files + endArgsMPMac)
            else:
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_5.so', algorithm_name + os.sep + 'benchmark_pi.c'] + c_files + endArgsMP)
            cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_5.so')
            results['small_MP'] = {}
            results['small_MP']['min_throughput_encrypting'], results['small_MP']['max_throughput_encrypting'], results['small_MP']['average_throughput_encrypting'], results['small_MP']['min_throughput_decrypting'], results['small_MP']['max_throughput_decrypting'], results['small_MP']['average_throughput_decrypting'], results['small_MP']['encrypting_times'], results['small_MP']['decrypting_times'] = benchmark_small(cBenchmark, pi)
            executable_size += os.path.getsize(executable_folder + os.sep + 'executable_5.so')
            executable_num += 1
            os.system('rm ' + executable_folder + os.sep + 'executable_5.so')
            
        os.system('rm ' + algorithm_name + os.sep + 'benchmark_pi.c')
        os.system('rm ' + executable_folder + os.sep + 'executable_3.so')
        os.system('rm ' + executable_folder + os.sep + 'executable_4.so')

    else:
        
        print("Benchmarking 1 KB to 2 MB with optimisation")
        if not hamilton:
            os.system('cp benchmark_small.c ' + algorithm_name + os.sep + 'benchmark_small.c')
            subprocess.run(startArgs + [executable_folder + os.sep + 'executable_3.so', algorithm_name + os.sep + 'benchmark_small.c'] + c_files + endArgsSIMD)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_3.so')
        results['small_O2'] = {}
        results['small_O2']['min_throughput_encrypting'], results['small_O2']['max_throughput_encrypting'], results['small_O2']['average_throughput_encrypting'], results['small_O2']['min_throughput_decrypting'], results['small_O2']['max_throughput_decrypting'], results['small_O2']['average_throughput_decrypting'], results['small_O2']['encrypting_times'], results['small_O2']['decrypting_times'] = benchmark_small(cBenchmark, False)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_3.so')
        executable_num += 1
        
        print("Benchmarking 1 KB to 2 MB without optimisation")
        if not hamilton:
            subprocess.run(startArgs + [executable_folder + os.sep + 'executable_4.so', algorithm_name + os.sep + 'benchmark_small.c'] + c_files + endArgsNoOpt)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_4.so')
        results['small_O0'] = {}
        results['small_O0']['min_throughput_encrypting'], results['small_O0']['max_throughput_encrypting'], results['small_O0']['average_throughput_encrypting'], results['small_O0']['min_throughput_decrypting'], results['small_O0']['max_throughput_decrypting'], results['small_O0']['average_throughput_decrypting'], results['small_O0']['encrypting_times'], results['small_O0']['decrypting_times'] = benchmark_small(cBenchmark, False)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_4.so')
        executable_num += 1
        
        if not original:
            print("Benchmarking 1 KB to 2 MB with optimisation and openmp")
            if not hamilton:
                if platform.system() == 'Darwin':
                    subprocess.run(startArgs + [executable_folder + os.sep + 'executable_5.so', algorithm_name + os.sep + 'benchmark_small.c'] + c_files + endArgsMPMac)
                else:
                    subprocess.run(startArgs + [executable_folder + os.sep + 'executable_5.so', algorithm_name + os.sep + 'benchmark_small.c'] + c_files + endArgsMP)
            cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_5.so')
            results['small_MP'] = {}
            results['small_MP']['min_throughput_encrypting'], results['small_MP']['max_throughput_encrypting'], results['small_MP']['average_throughput_encrypting'], results['small_MP']['min_throughput_decrypting'], results['small_MP']['max_throughput_decrypting'], results['small_MP']['average_throughput_decrypting'], results['small_MP']['encrypting_times'], results['small_MP']['decrypting_times'] = benchmark_small(cBenchmark, False)
            executable_size += os.path.getsize(executable_folder + os.sep + 'executable_5.so')
            executable_num += 1
            if not hamilton:
                os.system('rm ' + executable_folder + os.sep + 'executable_5.so')
        if not hamilton:  
            os.system('rm ' + algorithm_name + os.sep + 'benchmark_small.c')
            os.system('rm ' + executable_folder + os.sep + 'executable_3.so')
            os.system('rm ' + executable_folder + os.sep + 'executable_4.so')
        
    # Run a benchmark for latency that runs all algorithms with no input to encrypt to get an idea of the algorithms overhead
    
    print("Benchmarking empty test vectors with optimisation")
    if not hamilton:
        os.system('cp benchmark_latency.c ' + algorithm_name + os.sep + 'benchmark_latency.c')
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable_6.so', algorithm_name + os.sep + 'benchmark_latency.c'] + c_files + endArgsSIMD)
    cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_6.so')
    results['latency_O2'] = {}
    results['latency_O2']['latency_encryption'], results['latency_O2']['latency_decryption'] = benchmark_latency(cBenchmark)
    executable_size += os.path.getsize(executable_folder + os.sep + 'executable_6.so')
    executable_num += 1
    
    print("Benchmarking empty test vectors without optimisation")
    if not hamilton:
        subprocess.run(startArgs + [executable_folder + os.sep + 'executable_7.so', algorithm_name + os.sep + 'benchmark_latency.c'] + c_files + endArgsNoOpt)
    cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_7.so')
    results['latency_O0'] = {}
    results['latency_O0']['latency_encryption'], results['latency_O0']['latency_decryption'] = benchmark_latency(cBenchmark)
    executable_size += os.path.getsize(executable_folder + os.sep + 'executable_7.so')
    executable_num += 1
    
    if not original:
        print("Benchmarking empty test vectors with optimisation and openmp")
        if not hamilton:
            if platform.system() == 'Darwin':
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_8.so', algorithm_name + os.sep + 'benchmark_latency.c'] + c_files + endArgsMPMac)
            else:
                subprocess.run(startArgs + [executable_folder + os.sep + 'executable_8.so', algorithm_name + os.sep + 'benchmark_latency.c'] + c_files + endArgsMP)
        cBenchmark = CDLL('./' + executable_folder + os.sep + 'executable_8.so')
        results['latency_MP'] = {}
        results['latency_MP']['latency_encryption'], results['latency_MP']['latency_decryption'] = benchmark_latency(cBenchmark)
        executable_size += os.path.getsize(executable_folder + os.sep + 'executable_8.so')
        executable_num += 1
        if not hamilton:
            os.system('rm ' + executable_folder + os.sep + 'executable_8.so')
    if not hamilton:
        os.system('rm ' + algorithm_name + os.sep + 'crypto_aead.h')
        os.system('rm ' + algorithm_name + os.sep + 'benchmark_latency.c')
        os.system('rm ' + executable_folder + os.sep + 'executable_6.so')
        os.system('rm ' + executable_folder + os.sep + 'executable_7.so')
        os.system('rm -rf ' + executable_folder)
    
    results["average rom usage"] = executable_size / executable_num
    return results

def upload_data(algorithm_name, result):
    
    folderItems = []
    for filename in sorted(os.listdir(algorithm_name)):
        if(filename[-1] == 'c' or filename[-1] == 'h'):
            with open(algorithm_name + os.sep + filename) as file:
                folderItems.append(file.read())
    
    resp = requests.post('https://2zipnoysp5.execute-api.eu-west-1.amazonaws.com/prod/hash',
                         data=json.dumps({'folderItems':folderItems, 'result':result}),
                         headers={'Content-Type':'application/json'})
    resultHash = resp.json()['resultHash']
    print(resultHash)
    body = {'resultHash': resultHash, 'result': result}

    resp = requests.post('https://2zipnoysp5.execute-api.eu-west-1.amazonaws.com/prod/add',
                         data=json.dumps(body),
                         headers={'Content-Type':'application/json'})
    print(resp.status_code)
    
    return True

if __name__ == '__main__':

    parser = argparse.ArgumentParser('This tool ought to help you contribute to the online benchmarking of LWC algorithms')

    parser.add_argument('--algorithm_name', type=str, 
        help='the name of the algorithm submitted to the online verifier which also shares the name of the subdirectory containing the files')

    parser.add_argument('--upload', type=bool, default=False, 
        help='whether or not to upload the results of the benchmark')

    parser.add_argument('--original', type=bool, default=False, 
        help='if the execution is on an original implementation, the benchmark skips some metrics')
    
    parser.add_argument('--pi', type=bool, default=False, 
        help='run fewer tests if raspberry pi')

    parser.add_argument('--precollected_data', type=bool, default=False, 
        help='have already collected the data')
    
    parser.add_argument('--data_filename', type=str,
        help='file name of the file containing the result data')

    parser.add_argument('--hamilton', type=bool, default=False, 
        help='data collection running on hamilton')

    parser.add_argument('--additional_information', type=str, default='no additional information given',
        help='provide others with additional information about your system such as CPU model and operating system version')        

    args = parser.parse_args()

    if args.precollected_data == True:
        with open(args.data_filename, 'r') as data_file:
            results = eval(data_file.read())
            #results['algorithm_name'] = 'elephant'
            print(results['algorithm_name'])
            upload_data(results['algorithm_name'], results)

    else:
        result = run_benchmark(args.algorithm_name, args.original, args.pi, args.hamilton)
        result['algorithm_name'] = args.algorithm_name
        result['additional_information'] = args.additional_information
        print(result)
        if args.hamilton:
            with open('result_' + result['algorithm_name'] + result['additional_information'], 'w') as f:
                f.write(json.dumps(result))
        elif args.upload:
            upload_data(args.algorithm_name, result)
