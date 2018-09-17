import numpy as np
import pandas as pd
import random
import ctypes as ct
import os, sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
dll = np.ctypeslib.load_library(os.path.join(BASE_DIR, 'sampler.so'), '.')
matchings = np.load('./matchings.npy')
sizes = np.load('./sizes.npy')
timeidx = np.load('./timeidx.npy')

c_match = matchings.ctypes.data_as(ct.c_void_p)
c_sizes = sizes.ctypes.data_as(ct.c_void_p)
c_timeidx = timeidx.ctypes.data_as(ct.c_void_p)

def init_heuristic():
  probs = np.zeros((303, 41), dtype=np.float32)
  for it in range(303):
    probs[it, : sizes[it]] = 1.0 / sizes[it]
  return probs

def monte_carlo(probs):
  c_probs = probs.ctypes.data_as(ct.c_void_p)
  assign = np.zeros((303), dtype=np.int32)
  c_assign = assign.ctypes.data_as(ct.c_void_p)
  dll.sampling(c_match, c_sizes, c_probs, c_assign)
  return assign

def greedy(probs):
  assign = monte_carlo(probs)
  ports = [[] for _ in range(69)]
  for it in range(69):
    where = np.where(assign == it)[0]
    ports[it] += where.tolist()
  sizes_arr = [len(item) for item in ports]
  # print(timeidx.shape, timeidx, timeidx.dtype)
  maxlen = max(sizes_arr)
  for it in range(69):
    while len(ports[it]) < maxlen:
      ports[it].append(-1)
  ports = np.array(ports, dtype=np.int32)
  sizes_arr = np.array(sizes_arr, dtype=np.int32)
  ans_arr = np.zeros((2), dtype=np.int32)
  c_ports = ports.ctypes.data_as(ct.c_void_p)
  c_sizes = sizes_arr.ctypes.data_as(ct.c_void_p)
  c_assign = assign.ctypes.data_as(ct.c_void_p)
  c_maxlen = ct.c_int(maxlen)
  c_ans = ans_arr.ctypes.data_as(ct.c_void_p)
  dll.greedy_search(c_ports, c_sizes, c_timeidx, c_maxlen, c_ans, c_assign)
  return ans_arr[0], assign


if __name__ == '__main__':
  probs = init_heuristic()
  for _ in range(64):
    objval, assign = greedy(probs)
    print(objval, end=' ')
  # print(assign.shape, assign.max(), assign.min())
  # print(assign[: 10])

