import networkx as nx
from networkx.algorithms import bipartite

class User():
    def __init__(self,userId):
      self.userId=userId
      
    def __key(self):
        return (self.userId)

    def __eq__(self,x, y):
        return x.__key() == y.__key()

    def __hash__(self):
        return hash(self.__key())
      
class Restaurant():
    def __init__(self,restaurantId):
      self.restaurantId=restaurantId
      
    def __key(self):
        return (self.restaurantId)

    def __eq__(self,x, y):
        return x.__key() == y.__key()

    def __hash__(self):
        return hash(self.__key())