#!/usr/bin/env python3

import sys
import os
import json

#import enum import Enum
#class VisualizationGridOptions(Enum):

from collections import namedtuple 
VisualizationGridOptions = namedtuple('VisualizationGridOptions', 'id,  mappingData, gridNumDimensions, posNumDimensions, gridDimensions, x0, x1' )

def writeVisusMesh(path, gridOptions):
    data = {}
    
    
    for grid in gridOptions:
        gridName = 'VisusMesh' + str(grid.id)
        data[gridName] = {}
        
        data[gridName]["gridNumDimensions"] = grid.gridNumDimensions
        data[gridName]["posNumDimensions"] = grid.posNumDimensions
        
        #data[gridName]["gridDimensions"] = grid.gridDimensions
        data[gridName]["gridDimensions"] = [i for i in grid.gridDimensions if i != 0]
        
        data[gridName]["mappingData"] = [] 
        for dataName in grid.mappingData:
            data[gridName]["mappingData"].append( dataName + str(grid.id) )

#        cellDim = \
#            (100.0 / data[gridName]["gridDimensions"][0], 100.0 / data[gridName]["gridDimensions"][1])


           
        vertices = []
        
#        for z in range(data[gridName]["gridDimensions"][1]):
        if ( grid.gridDimensions[2] == 0):
            cellDim = \
                (abs(grid.x0[0]-grid.x1[0]) / grid.gridDimensions[0], \
                abs(grid.x0[1]-grid.x1[1]) / grid.gridDimensions[1])
            assert( grid.x0[2] == grid.x1[2] )
            for y in range(data[gridName]["gridDimensions"][1]):
                for x in range(data[gridName]["gridDimensions"][0]):
                    vertices.append({
                        "idx" : y * data[gridName]["gridDimensions"][0] + x,
                        "pos" : [
                            (x + 0.5) * cellDim[0] + grid.x0[0],
                            (y + 0.5) * cellDim[1] + grid.x0[1],
                            grid.x0[2] ]
                        })
        elif ( grid.gridDimensions[1] == 0):
            cellDim = \
                (abs(grid.x0[0]-grid.x1[0]) / grid.gridDimensions[0], \
                abs(grid.x0[2]-grid.x1[2]) / grid.gridDimensions[2])
            assert( grid.x0[1] == grid.x1[1] )
            for z in range(grid.gridDimensions[2]):
                for x in range(grid.gridDimensions[0]):
                    vertices.append({
                        "idx" : z * grid.gridDimensions[0] + x,
                        "pos" : [
                            (x + 0.5) * cellDim[0] + grid.x0[0],
                            grid.x0[1],
                            (z + 0.5) * cellDim[1] + grid.x0[2]]
                        })
        elif ( grid.gridDimensions[0] == 0):
            cellDim = \
                (abs(grid.x0[1]-grid.x1[1]) / grid.gridDimensions[1], \
                abs(grid.x0[2]-grid.x1[2]) / grid.gridDimensions[2])
            assert( grid.x0[0] == grid.x1[0] )
            for z in range(grid.gridDimensions[2]):
                for y in range(grid.gridDimensions[1]):
                    vertices.append({
                        "idx" : z * grid.gridDimensions[1] + y,
                        "pos" : [
                            grid.x0[0],
                            (y + 0.5) * cellDim[0] + grid.x0[1] ,
                            (z + 0.5) * cellDim[1] + grid.x0[2] ]
                        })
        else:
            raise Exception("One of the spatial dimensions has to be zero!")
              
        edges = []
        idx = 0
        for y in range(data[gridName]["gridDimensions"][1]):
            for x in range(data[gridName]["gridDimensions"][0]):
                if (x < (data[gridName]["gridDimensions"][0] - 1)):
                    edges.append({
                        "idx" : idx,
                        "nodes" : [
                            y * data[gridName]["gridDimensions"][0] + x,
                            y * data[gridName]["gridDimensions"][0] + (x + 1)]})
                    idx = idx + 1
                if (y < (data[gridName]["gridDimensions"][1] - 1)):
                    edges.append({
                        "idx" : idx,
                        "nodes" : [
                            y * data[gridName]["gridDimensions"][0] + x,
                            (y + 1) * data[gridName]["gridDimensions"][0] + x]})
                    idx = idx + 1

        data[gridName]["vertices"] = vertices
        data[gridName]["edges"] = edges

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

    gridOptions = []
    gridOptions.append( VisualizationGridOptions( id=0, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[0, 3, 3], x0=[0.5, 0., 0.], x1=[0.5, 1., 1.] ) )
    gridOptions.append( VisualizationGridOptions( id=1, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[3, 0, 3], x0=[0., 0.5, 0.], x1=[1., 0.5, 1.] ) )
    gridOptions.append( VisualizationGridOptions( id=2, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[3, 3, 0], x0=[0., 0., 0.5], x1=[1., 1., .5] ) )
    
    gridOptions.append( VisualizationGridOptions( id=3, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[0, 4, 4], x0=[0.75, 0.5, 0.5], x1=[0.75, 1.0, 1.0] ) )
    gridOptions.append( VisualizationGridOptions( id=5, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[4, 0, 4], x0=[0.5, 0.75, 0.5], x1=[1.0, 0.75, 1.] ) )
    gridOptions.append( VisualizationGridOptions( id=4, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[4, 4, 0], x0=[0.5, 0.5, 0.75], x1=[1., 1., .75] ) )
    
    gridOptions.append( VisualizationGridOptions( id=7, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[0, 3, 3], x0=[0.625, 0.5, 0.5], x1=[0.625, 0.75, 0.75] ) )
    gridOptions.append( VisualizationGridOptions( id=6, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[3, 0, 3], x0=[0.5, 0.625, 0.5], x1=[.75, 0.625, .75] ) )
    gridOptions.append( VisualizationGridOptions( id=8, mappingData=['Concentration'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[3, 3, 0], x0=[0.5, 0.5, 0.625], x1=[.75, .75, .625] ) )
    
    
    #visGridOpt = VisualizationGridOptions( 'VisusGrid0', mappingData=['Concentration0'], gridNumDimensions=2, posNumDimensions=3, gridDimensions=[10, 10], x0=[0, 0, 0], x1=[1, 1, 1] )
    for gridOption in gridOptions:
      print( gridOption ) 


    writeVisusMesh(path, gridOptions)

    print('Wrote mesh file', path)
