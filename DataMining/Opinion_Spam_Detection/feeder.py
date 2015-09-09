
import networkx as nx
import numpy as np
import matplotlib.pyplot as plt
import pylab

from networkx.algorithms import bipartite
import numpy


def updateRank(seedNode,currentNode,pathLength):
   if pathLength==2 or pathLength==4:
     if currentNode not in rank:
         rank[currentNode]=[]
         #Below dict contains node at 1st neighbour
         rank[currentNode].append({})
         #Below dict contains node at 2nd neighbour
         rank[currentNode].append({})
   if pathLength==2:
    if currentNode in rank[seedNode][0]:
         rank[seedNode][0][currentNode]=  rank[seedNode][0][currentNode]+1
    else:
         rank[seedNode][0][currentNode]=1
    if currentNode in seed:
      return
    if seedNode in rank[currentNode][0]:
         rank[currentNode][0][seedNode]=  rank[currentNode][0][seedNode]+1
    else:
         rank[currentNode][0][seedNode]=1
    
   if pathLength==4:
     if currentNode in rank[seedNode][1]:
         rank[seedNode][1][currentNode]=  rank[seedNode][1][currentNode]+1
     else:
         rank[seedNode][1][currentNode]=1
     if currentNode in seed:
      return
     if seedNode in rank[currentNode][1]:
         rank[currentNode][1][seedNode]=  rank[currentNode][1][seedNode]+1
     else:
         rank[currentNode][1][seedNode]=1
def dfs(seedNode,currentNode,pathLength):
  if currentNode in visitedNodes:
    return
  updateRank(seedNode,currentNode,pathLength)
  visitedNodes.append(currentNode)
  succNodes=G.successors(currentNode)
  for succNode in succNodes:
    dfs(seedNode,succNode,pathLength+1)
  visitedNodes.remove(currentNode)
def generateM():
  alpha=0.75
  beta=0.25
  M={}
  nodes=rank.keys()
  for i in nodes:
    for j in nodes:
      M[i,j]=0
  #fill each column one by one
  for j in nodes:
    #First traverse at depth 0
    if rank[j][0]:
      nodesDict=rank[j][0]
      sum0=0
      if rank[j][1]:
        alpha=0.75
      else:
        alpha=1.0
      for val in nodesDict.values():
        sum0=sum0+val
      for key,val in nodesDict.items():
        ratio=val
        contribution=ratio/float(sum0) * alpha
        M[key,j]=M[key,j]+contribution
     #First traverse at depth 0
    if rank[j][1]:
      nodesDict=rank[j][1]
      sum1=0
      if rank[j][0]:
        beta=0.25
      else:
        beta=1.0
      for val in nodesDict.values():
        sum1=sum1+val
      for key,val in nodesDict.items():
        ratio=val
        contribution=ratio/float(sum1) * beta
        M[key,j]=M[key,j]+contribution
  return M
'''  

rank={'A':[{'B':2,C:'3'},{}]}
'''
seed=['A','B']
G = nx.Graph()
G.add_nodes_from(['A','B','C','D','E','F'], bipartite=0) # Add the node attribute "bipartite"
G.add_nodes_from([1,2,3,4,5,6,7,8], bipartite=1)
G.add_edges_from([('A', 1),('A',2),('A',3),('A',4),('A',5)])
G.add_edges_from([('B',3),('B',4),('B',8),('B',2)])
G.add_edges_from([('C',8)])
G.add_edges_from([('D',8),('D',7)])
G.add_edges_from([('E',5),('E',6)])
G.add_edges_from([('F',8),('F',7)])
G=G.to_directed()
rank={}
visitedNodes=[]
for seedNode in seed:
     rank[seedNode]=[]
     #Below dict contains node at 1st neighbour
     rank[seedNode].append({})
     #Below dict contains node at 2nd neighbour
     rank[seedNode].append({})
for seedNode in seed:
  dfs(seedNode,seedNode,0)
print rank
MDict=generateM()
print MDict
nodes=rank.keys()
indexNodes={}
count=0
for node in nodes:
    indexNodes[node]=count
    count=count+1
MList=[[0 for x in range(count)] for x in range(count)] 
for key,val in MDict.items():
    i=key[0]
    j=key[1]
    iIndex=indexNodes[i]
    jIndex=indexNodes[j]
    MList[iIndex][jIndex]=MDict[i,j]
M=numpy.array(MList) 
print M
print indexNodes