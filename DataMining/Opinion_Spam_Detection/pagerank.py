import numpy


def matrix_factorization(R, P, Q, K, steps=15, alpha=0.0002, beta=0.02):
    
    for step in xrange(steps):
        for i in xrange(len(R)):
            for j in xrange(len(R[i])):
                if R[i][j] > 0:
                    eij = R[i][j] - numpy.dot(P[i,:],Q[:,j])
                    for k in xrange(K):
                        P[i][k] = P[i][k] + alpha * (2 * eij * Q[k][j] - beta * P[i][k])
                        Q[k][j] = Q[k][j] + alpha * (2 * eij * P[i][k] - beta * Q[k][j])
        eR = numpy.dot(P,Q)
        e = 0
        for i in xrange(len(R)):
            for j in xrange(len(R[i])):
                if R[i][j] > 0:
                    e = e + pow(R[i][j] - numpy.dot(P[i,:],Q[:,j]), 2)
                    for k in xrange(K):
                        e = e + (beta/2) * ( pow(P[i][k],2) + pow(Q[k][j],2) )
        if e < 0.001:
            break
    return P, Q
  
def personalizedPageRank(R,M,S,steps=40,tolerance=0.002):
  alpha=0.85
  nodes=len(R)
  for step in range(steps):
    oldR=R
    left=alpha*numpy.dot(M,oldR)
    right=(1-alpha)*S
    newR=left+right
    sum=0.0
    for node in range(nodes):
      sum=sum+abs(newR[node]-oldR[node])
    R=newR  
    if sum<nodes*tolerance:
      break
  return R

M= [[0.5, 0.25, 0.125, 0.125],
         [0.5, 0.25, 0.125, 0.125],
        [0.5, 0.25, 0.125, 0.125],
         [0.5, 0.25, 0.125, 0.125]]
M = numpy.array(M)
R=[[0.2],[0.6],[0.5],[0.4]]
R = numpy.array(R)
S=[[0.5],[0],[0],[0.5]]
S = numpy.array(S)

R=personalizedPageRank(R,M,S)
print R
print 'HELL RAISEDS OVER'