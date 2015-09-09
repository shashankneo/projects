import numpy
from time import time
"""
@INPUT:
    R     : a matrix to be factorized, dimension N x M
    P     : an initial matrix of dimension N x K
    Q     : an initial matrix of dimension M x K
    K     : the number of latent features
    steps : the maximum number of steps to perform the optimisation
    alpha : the learning rate
    beta  : the regularization parameter
@OUTPUT:
    the final matrices P and Q
"""
def matrix_factorization(R, P, Q, K, steps=5000, alpha=0.0002, beta=0.02):
    
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

###############################################################################

if __name__ == "__main__":
  

    n = 4
    r = 3
    f = 4
    """
    G = numpy.random.rand(n,r)
    F = numpy.random.rand(r,f)
    U = numpy.random.rand(n,f)
    """
    G = numpy.random.rand(n,r)
    
    print "Initial G:"
    print G
    
    F = [[1, 0.7, 0.7, 0.5],
         [0.2, 0.2, 0.2, 1],
         [0.1, 0.3, 0.3, 0.3]]
    F = numpy.array(F)
    
    U = [[0.8, 0.7, 0.8, 0],
         [0.1, 0.7, 0.8, 0],
         [0, 0.2, 0.2, 0],
         [0, 0, 0, 1]
        ]
    U = numpy.array(U) 
        
    print "F: "
    print F
    print "U:"
    print U     
    t0 = time();
    tG, tF = matrix_factorization(U, G, F, r)
    
    print "tG: "
    print tG
    print "tF: "
    print tF
    
    tU = numpy.dot(tG,tF)
    print "final tU"
    print tU
    t1=time();
    print " time used: %f" % (t1 - t0), "secs.\n";