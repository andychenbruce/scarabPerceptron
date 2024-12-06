#!/usr/bin/env python3
import os
import json
import argparse
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import csv
import numpy
from matplotlib import cm

hists = [4, 8, 16, 24, 30]
perceptron_nums = [1, 4, 8, 16, 32, 128, 1024, 32768, 131072]

def read_descriptor_from_json(descriptor_filename):
    # Read the descriptor data from a JSON file
    with open(descriptor_filename, 'r') as json_file:
        descriptor_data = json.load(json_file)
    return descriptor_data

descriptor = read_descriptor_from_json("./scarab/test.json")
workloads = descriptor["workloads_list"]
experiment = descriptor["experiment"]

def get_config_data(config_name: str):
    paths = [f"./exp/simulations/{workload}/{experiment}/{config_name}" for workload in workloads]
    data_ipc = numpy.mean([get_path_data(path, "Periodic IPC") for path in paths])
    data_mispredict = numpy.mean([get_path_data(path, "BP_ON_PATH_MISPREDICT_pct") for path in paths]) / 100
    return data_ipc, data_mispredict

def get_path_data(path: str, label: str):
    bp = os.path.join(path, "bp.stat.0.csv")
    mispredict_pct = None
    with open(bp, 'r') as bp_file:
        for line in bp_file:
            tokens = [x.strip() for x in line.split(',')]
            if(tokens[0] == label):
                mispredict_pct = float(tokens[1])
    return mispredict_pct


def plot_matrix(matrix, title, y_labels, filename):
    fig, axes = plt.subplots()
    im = axes.imshow(matrix)
    plt.xlabel("global history bits")
    plt.ylabel("number of perceptrons")
    plt.xticks(range(len(hists)), labels=hists)
    
    plt.yticks(range(len(perceptron_nums)), labels=y_labels)
    plt.title(title)
    fig.colorbar(im, ax=axes, orientation='vertical', fraction=0.046, pad=0.04)
    fig.set_tight_layout(True)
    plt.savefig(filename)

                
def plot_matrixes(config_datas):
    matrix = numpy.array([[config_datas[f"perceptron_hist_{hist}_num_{perceptron_num}"] for hist in hists] for perceptron_num in perceptron_nums])
    tagescl = numpy.array([config_datas[f"tagescl_hist_{hist}"] for hist in hists])
    gshare = numpy.array([config_datas[f"gshare_hist_{hist}"] for hist in hists])
    plot_matrix(matrix[:, :, 0], "IPC", perceptron_nums, "matrix_ipc_all.png")
    plot_matrix(numpy.concatenate([[gshare[:, 0]], matrix[1:, :, 0]]), "IPC", ["gshare"] + perceptron_nums[1:], "matrix_ipc_gshare.png")
    plot_matrix(numpy.concatenate([[tagescl[:, 0]], matrix[1:, :, 0]]), "IPC", ["tagescl"] + perceptron_nums[1:], "matrix_ipc_tagescl.png")
    plot_matrix(matrix[:, :, 1], "Mispredict Ratio", perceptron_nums, "matrix_mispred_all.png")
    plot_matrix(numpy.concatenate([[gshare[:, 1]], matrix[1:, :, 1]]), "Mispredict Ratio", ["gshare"] + perceptron_nums[1:], "matrix_mispred_gshare.png")
    plot_matrix(numpy.concatenate([[tagescl[:, 1]], matrix[1:, :, 1]]), "Mispredict Ratio", ["tagescl"] + perceptron_nums[1:], "matrix_mispred_tagescl.png")
    
def plot_x_axis_num_perceptrons(config_datas):
    matrix = numpy.array([[config_datas[f"perceptron_hist_{hist}_num_{perceptron_num}"] for hist in hists] for perceptron_num in perceptron_nums])[:, :, 1]
    for skip_end, filename in zip([-2, -1, len(perceptron_nums)], ["x_num_perceptrons_no_last_2.png", "x_num_perceptrons_no_last.png", "x_num_perceptrons_all.png"]):
        fig, axes = plt.subplots()
        plt.xlabel("Number of perceptrons")
        plt.ylabel("miss_rate")
        for i in range(len(hists)):
            plt.plot(perceptron_nums[1:skip_end], matrix.T[i][1:skip_end], label=f'history size {hists[i]}')
        plt.legend(loc='lower right')
        plt.savefig(filename)

def plot_x_axis_history(config_datas):
    matrix = numpy.array([[config_datas[f"perceptron_hist_{hist}_num_{perceptron_num}"] for hist in hists] for perceptron_num in perceptron_nums])[:, :, 1]
    tagescl = numpy.array([config_datas[f"tagescl_hist_{hist}"] for hist in hists])[:, 1]
    gshare = numpy.array([config_datas[f"gshare_hist_{hist}"] for hist in hists])[:, 1]
    
    fig, axes = plt.subplots()
    plt.xticks(hists, labels=hists)
    plt.xlabel("History bits")
    plt.ylabel("mispredict ratio (log)")
    plt.plot(hists, numpy.log(tagescl / 100.0), label=f'tagescl')
    plt.plot(hists, numpy.log(gshare / 100.0), label=f'gshare')
    for i in range(len(perceptron_nums)):
        plt.plot(hists, numpy.log(matrix[i] / 100.0), label=f'{perceptron_nums[i]}')
    plt.legend(loc='upper left', bbox_to_anchor=(0.8, 1.1),)
    plt.savefig("x_hist.png")
             

def plot_metrics():
    configs = descriptor["configurations"]
    config_datas = {config: get_config_data(config) for config in configs}
    plot_matrixes(config_datas)
    plot_x_axis_num_perceptrons(config_datas)
    plot_x_axis_history(config_datas)

plot_metrics()
