#!/usr/bin/env python3

import sys
import os
import json

def writeVisusMesh(path):
    data = {}
    data["numDimensions"] = 2
    data["gridDimensions"] = [10, 10]

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
