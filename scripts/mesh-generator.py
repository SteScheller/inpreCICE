#!/usr/bin/env python3

import sys
import os
import json
import argparse
import itertools
import numpy as np
from typing import Tuple, List, Dict, Union

def writeVisMeshes(path, meshes):
    with open(path, 'w') as f:
        json.dump(meshes, f, indent=4)

def makeRegularPlane3D(
        p: np.ndarray,
        u: np.ndarray,
        v: np.ndarray,
        gridSize: Tuple[int, int]
        ) -> Tuple[
            List[Dict[str, Union[int, List[float]]]],
            List[Dict[str, Union[int, List[float]]]]]:
    vertices = list()
    edges = list()

    mx, my = u / (gridSize[0] - 1), v / (gridSize[1] - 1)
    idxEdge = 0
    for iy, ix in itertools.product(range(gridSize[1]), range(gridSize[0])):
        vertices.append({
            'idx' : iy * gridSize[0] + ix,
            'pos' : list(p + ix * mx + iy * my)})
        if (ix < (gridSize[0] - 1)):
            edges.append({
                'idx' : idxEdge,
                'nodes' : [
                    iy * gridSize[0] + ix,
                    iy * gridSize[0] + (ix + 1)]})
            idxEdge = idxEdge + 1
        if (iy < (gridSize[1] - 1)):
            edges.append({
                'idx' : idxEdge,
                'nodes' : [
                    iy * gridSize[0] + ix,
                    (iy + 1) * gridSize[0] + ix]})
            idxEdge = idxEdge + 1

    return vertices, edges

def generateCase1():
    data = {
            'gridNumDimensions' : 2,
            'posNumDimensions' : 3,
            'gridDimensions' : [10, 10] }
    vertices, edges = makeRegularPlane3D(
            np.array([5, 5, 77]),
            np.array([90, 0, -54]),
            np.array([0, 90, 0]),
            data['gridDimensions'])
    data.update(vertices=vertices, edges=edges)

    return data

def generateCase2():
    data = dict()

    gridDim = [10, 10]
    for i in range(9):
        frac = {
            'gridNumDimensions' : 2,
            'posNumDimensions' : 3,
            'gridDimensions' : gridDim}
        data.update({'frac' + str(i) : frac})

    vertices, edges = makeRegularPlane3D(
            np.array([0.5, 0, 0]),
            np.array([0, 1, 0]),
            np.array([0, 0, 1]),
            gridDim)
    data['frac0'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0, 0.5, 0]),
            np.array([1, 0, 0]),
            np.array([0, 0, 1]),
            gridDim)
    data['frac1'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0, 0, 0.5]),
            np.array([1, 0, 0]),
            np.array([0, 1, 0]),
            gridDim)
    data['frac2'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.75, 0.5, 0.5]),
            np.array([0, 0.5, 0]),
            np.array([0, 0, 0.5]),
            gridDim)
    data['frac3'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.5, 0.5, 0.75]),
            np.array([0.5, 0, 0]),
            np.array([0, 0.5, 0]),
            gridDim)
    data['frac4'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.5, 0.75, 0.5]),
            np.array([0.5, 0, 0]),
            np.array([0, 0, 0.5]),
            gridDim)
    data['frac5'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.5, 0.625, 0.5]),
            np.array([0.25, 0, 0]),
            np.array([0, 0, 0.25]),
            gridDim)
    data['frac6'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.625, 0.5, 0.5]),
            np.array([0, 0.25, 0]),
            np.array([0, 0, 0.25]),
            gridDim)
    data['frac7'].update(vertices=vertices, edges=edges)

    vertices, edges = makeRegularPlane3D(
            np.array([0.5, 0.5, 0.625]),
            np.array([0.25, 0, 0]),
            np.array([0, 0.25, 0]),
            gridDim)
    data['frac8'].update(vertices=vertices, edges=edges)

    return data

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description=(
            'Generate a json-file describing the meshes used for '
            'the visualization of the different fracture benchmark cases.'))
    parser.add_argument(
        metavar='output-file',
        type=str,
        dest='file',
        help='path where the generated json-file is written to')

    args = parser.parse_args()

    # get target path
    if not os.path.exists(os.path.dirname(args.file)):
        print('Error: output directory does not exist!')
        exit(1)

    if os.path.exists(args.file):
        text = input('Overwrite ' + args.file + ' ? [y/N] ')
        if text.lower() not in ['yes', 'y']:
            exit(0)

    meshes = dict()
    meshes['case1'] = generateCase1()
    meshes['case2'] = generateCase2()

    writeVisMeshes(args.file, meshes)
    print('Wrote mesh file', args.file)

