#!/usr/bin/env python3

import sys
import os
import json

def writeVisusMesh(path):
    data = {}
    data["gridNumDimensions"] = 2
    data["posNumDimensions"] = 3
    data["gridDimensions"] = [10, 10]

    cellDim = \
        (100.0 / data["gridDimensions"][0], 100.0 / data["gridDimensions"][1])
    vertices = []
    for y in range(data["gridDimensions"][1]):
        for x in range(data["gridDimensions"][0]):
            vertices.append({
                "idx" : y * data["gridDimensions"][0] + x,
                "pos" : [
                    (x + 0.5) * cellDim[0],
                    (y + 0.5) * cellDim[1],
                    -0.6 * (x + 0.5) * cellDim[0] + 80.0]
                })
    edges = []
    idx = 0
    for y in range(data["gridDimensions"][1]):
        for x in range(data["gridDimensions"][0]):
            if (x < (data["gridDimensions"][0] - 1)):
                edges.append({
                    "idx" : idx,
                    "nodes" : [
                        y * data["gridDimensions"][0] + x,
                        y * data["gridDimensions"][0] + (x + 1)]})
                idx = idx + 1
            if (y < (data["gridDimensions"][1] - 1)):
                edges.append({
                    "idx" : idx,
                    "nodes" : [
                        y * data["gridDimensions"][0] + x,
                        (y + 1) * data["gridDimensions"][0] + x]})
                idx = idx + 1

    data["vertices"] = vertices
    data["edges"] = edges

    with open(path, 'w') as f:
        json.dump(data, f,  indent=4)

if __name__ == '__main__':
    # get target path
    path = './visus-mesh.json'
    if len(sys.argv) >= 2:
        path = sys.argv[1]
        if not os.path.exists(os.path.dirname(path)):
            print('Error: output directory does not exist!')
            exit(1)
        path = os.path.normpath(text)

    if os.path.exists(path):
        text = input('Overwrite ' + path + ' ? [y/N] ')
        if text not in ['yes', 'y', 'Yes', 'Y']:
            exit(0)

    writeVisusMesh(path)

    print('Wrote mesh file', path)
