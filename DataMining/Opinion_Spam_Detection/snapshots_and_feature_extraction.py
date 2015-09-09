import pymongo
import datetime
import ast
import networkx as nx
import os
from networkx.algorithms import bipartite
from collections import Counter
from cosine_similarity import get_cosine, text_to_vector
import pymf
import numpy
from sort_data import allReviews_ts
from time import time
import operator
import matplotlib.pyplot as plt

class SpamDetection:

    con = pymongo.Connection()
    db = con.dm_proj
    db_handle = db.reviews
    db_handle_sorted = db.sorted_reviews
    
    all_reviews = db_handle_sorted.find()
    
    restaurants_list = []
    reviewers_list = []
    num_roles = 3
    num_nodes = 0
    
    spam_users_ts = []
    
    F = [[0.799, 0.65, 0.65, 0.6],
         [0.200, 0.30, 0.30, 0.3],
         [0.001, 0.05, 0.05, 0.1]]
    
    
    #Declare Graph
    B= nx.Graph()
    
    def float_precision(self, a):
        a = int((a * 10000) + 0.5) / 10000.0 # Adding 0.5 rounds it up
        
        if a==0:
            a=0.0001
        return a
    
    def personalizedPageRank(self,R,M,S,steps=100):
        alpha=0.9
        nodes=len(R)
        tolerance=numpy.finfo(numpy.float).eps
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
    
    def generateM(self):
        alpha=0.75
        beta=0.25
        M={}
        nodes=self.rank.keys()
        for i in nodes:
            for j in nodes:
                M[i,j]=0
        #fill each column one by one
        for j in nodes:
        #First traverse at depth 0
            if self.rank[j][0]:
                nodesDict=self.rank[j][0]
                sum0=0
                if self.rank[j][1]:
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
            if self.rank[j][1]:
                nodesDict=self.rank[j][1]
                sum1=0
                if self.rank[j][0]:
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
    
    def updateRank(self,seedNode,currentNode,pathLength):
        if pathLength==2 or pathLength==4:
            if currentNode not in self.rank:
                self.rank[currentNode]=[]
                #Below dict contains node at 1st neighbour
                self.rank[currentNode].append({})
                #Below dict contains node at 2nd neighbour
                self.rank[currentNode].append({})
        if pathLength==2:
            if currentNode in self.rank[seedNode][0]:
                self.rank[seedNode][0][currentNode]=  self.rank[seedNode][0][currentNode]+1
            else:
                self.rank[seedNode][0][currentNode]=1
            if currentNode in self.seed:
                return
            if seedNode in self.rank[currentNode][0]:
                self.rank[currentNode][0][seedNode]=  self.rank[currentNode][0][seedNode]+1
            else:
                self.rank[currentNode][0][seedNode]=1
        
        if pathLength==4:
            if currentNode in self.rank[seedNode][1]:
                self.rank[seedNode][1][currentNode]=  self.rank[seedNode][1][currentNode]+1
            else:
                self.rank[seedNode][1][currentNode]=1
            if currentNode in self.seed:
                return
            if seedNode in self.rank[currentNode][1]:
                self.rank[currentNode][1][seedNode]=  self.rank[currentNode][1][seedNode]+1
            else:
                self.rank[currentNode][1][seedNode]=1
             
    def dfs(self,seedNode,currentNode,pathLength):
        if currentNode in self.visitedNodes:
            return
        if pathLength>4:
            return
        self.updateRank(seedNode,currentNode,pathLength)
        self.visitedNodes.append(currentNode)
        succNodes=self.G.successors(currentNode)
        for succNode in succNodes:
            self.dfs(seedNode,succNode,pathLength+1)
           
        self.visitedNodes.remove(currentNode)
        
    
    def prediction(self,newUsers,ts):
        self.seed=newUsers
        self.G=self.B.to_directed()
        self.rank={}
        self.visitedNodes=[]
        for seedNode in self.seed:
            seedNode=str(seedNode)
            self.rank[seedNode]=[]
            #Below dict contains node at 1st neighbour
            self.rank[seedNode].append({})
            #Below dict contains node at 2nd neighbour
            self.rank[seedNode].append({})
            
        for seedNode in self.seed:
            seedNode=str(seedNode)
            self.dfs(seedNode,seedNode,0)
        MDict=self.generateM()
        RList=[]

        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/graph_connectivity.txt")
        f = open(filenm, 'w')        
        f.write(str(self.rank)+"\n")
        f.close() 
        
        print "Prediction 1"
        
        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/seed_nodes.txt")
        f = open(filenm, 'w')
        for s in self.seed:
            f.write(str(s)+"\n")
        f.close() 
        
        #print MDict
        neighbour_nodes=self.rank.keys()
        seed_index = []
        indexNodes={}
        count=0
        
        
        print "Prediction 2"
        
        for node in neighbour_nodes:
            indexNodes[node]=count
            if node in self.seed:
                seed_index.append(count)
            RList.append([self.B.node[node]['spamicity_score']])
            count=count+1
        MList=[[0 for x in range(count)] for x in range(count)] 
        for key,val in MDict.items():
            i=key[0]
            j=key[1]
            iIndex=indexNodes[i]
            jIndex=indexNodes[j]
            MList[iIndex][jIndex]=MDict[i,j]
        M=numpy.array(MList)
        R = numpy.array(RList)
        dict_r={}
        
        
        print "Prediction 3"
        
        for x in range(0,len(R)):
            if x in seed_index: 
                dict_r[x]=R[x]
        sorted_dict_r = sorted(dict_r.items(), key=operator.itemgetter(1))
        
        cnt_r =0
        k = int(len(self.seed)*0.5)
       
        S =[]
        
        for x in range(0,len(neighbour_nodes)):
            S.append([0])
        
        for x in range(0,k):
            pos = sorted_dict_r[x][0]
            S[pos][0] = -1*(1/(float)(k))
        
        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/R_matrix.txt")
        f = open(filenm, 'w')
        
        indexNodes_swapped = {}
        
        for key,val in indexNodes.items():
            indexNodes_swapped[val] = key
    
        dict_r={}
        
        
        print "Prediction 4"
        
        for x in range(0,len(R)):
               dict_r[x]=R[x]
        sorted_dict_r = sorted(dict_r.items(), key=operator.itemgetter(1))     
        #print str(indexNodes_swapped)
        for x in range(0,len(sorted_dict_r)):
            pos = sorted_dict_r[x][0]
            s={"reviewerId":indexNodes_swapped[pos], "rank": self.float_precision(sorted_dict_r[x][1])}
            f.write(str(s)+"\n") 
            
        S = numpy.array(S)  
        
        t0 = time()
        R=self.personalizedPageRank(R,M,S)
        t1=time();
        print "PPR done in : %f" % (t1 - t0), "secs.\n";  
    
        #Print PPR
        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/R_matrix_after_PPR.txt")
        f = open(filenm, 'w')
        
        dict_ppr = {}
        
        for x in range(0,len(R)):
                dict_ppr[x]=R[x]
        sorted_dict_ppr = sorted(dict_ppr.items(), key=operator.itemgetter(1))
        
        least_val = sorted_dict_ppr[0][1]
        
        
        print "Prediction 5"
        
        #print str(indexNodes_swapped)
        
        for x in range(0,len(sorted_dict_r)):
            pos = sorted_dict_ppr[x][0]
            sa={"reviewerId":indexNodes_swapped[pos], "rank": self.float_precision(sorted_dict_ppr[x][1]+(-1*least_val))}
            f.write(str(sa)+"\n") 
        

        """
        for key,val in indexNodes.items():
            r = R[val][0]
            s={"reviewerId":key, "rank": self.float_precision(r)}
            f.write(str(s)+"\n")
        """    
        f.close()
        self.prediction_result(R,indexNodes,ts)
        
    
    def prediction_result(self,R,indexNodes,ts):
        dir_name = "snapshots/snapshot"+str(ts)
        filenm = dir_name+str("/accuracy.txt")
        f = open(filenm, 'w')
        
        for x in self.spam_users_ts:
            r = indexNodes[x]
            s={"reviewerId":x, "rank": self.float_precision(R[r])}
            f.write(str(s)+"\n")
            
        f.close()        
                 
    
    def apply_nmf(self, ts):
        self.get_bipartite_sets()
        nodes = self.reviewers_list
        
        U = []
        self.F = numpy.array(self.F)
        for n in nodes:
            l=[]
            l.append(self.B.node[n]['burstiness'])
            l.append(self.B.node[n]['rate_deviation'])
            l.append(self.B.node[n]['content_similarity'])
            l.append(self.B.node[n]['first_time'])
            U.append(l)
        
        U = numpy.array(U)
        
        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/U_matrix.txt")
        f = open(filenm, 'w')
        for i in range(0,len(nodes)):
            x = {"reviewerID": str(nodes[i]), "burstiness" :self.float_precision(U[i][0]), "rate_deviation":self.float_precision(U[i][1]),"content_similarity":self.float_precision(U[i][2]),"first_time":self.float_precision(U[i][3])}
            f.write(str(x)+"\n")
        f.close()
        nmf_mdl = pymf.NMF(U, num_bases=3)
        nmf_mdl.H = self.F
        
        print "NMF 1"
        nmf_mdl.factorize( niter=1000, compute_h=False, show_progress=True)
        
        print "NMF 2"
        dir_name = "snapshots/snapshot"+str(ts)
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        filenm = dir_name+str("/G_matrix.txt")
        f = open(filenm, 'w')
        for i in range(0,len(nodes)):
            score_1=0
            score_2=0
            if ts>=3:
                dir_name1 = "snapshots/snapshot"+str(ts-1)
                filenm = dir_name1+str("/G_matrix.txt")
                lines = [line.strip() for line in open(filenm)]
                for user in lines:
                    ts_1 = ast.literal_eval(user)
                    if ts_1['reviewerID']==nodes[i]:
                       score_1=ts_1['spam_score_ts']
                 
                dir_name2 = "snapshots/snapshot"+str(ts-2)
                filenm = dir_name1+str("/G_matrix.txt")
                lines = [line.strip() for line in open(filenm)]
                for user in lines:
                   ts_2 = ast.literal_eval(user)
                   if ts_2['reviewerID']==nodes[i]:
                       score_2=ts_2['spam_score_ts']    
            
            spamicity_score =self.float_precision((8*nmf_mdl.W[i][0]+nmf_mdl.W[i][1]-10*nmf_mdl.W[i][2])/3)
            spamicity_score_total = (spamicity_score+score_1+score_2)/3
                
            uid = nodes[i]
            self.B.node[uid]['spamicity_score'] = spamicity_score_total
            x = {"reviewerID": str(uid), "spam":self.float_precision(nmf_mdl.W[i][0]) ,"poss_spam":self.float_precision(nmf_mdl.W[i][1]) ,"not_spam":self.float_precision(nmf_mdl.W[i][2]),"spam_score_ts":self.float_precision(spamicity_score), "spam_score_total":self.float_precision(spamicity_score_total)}
            f.write(str(x)+"\n")
        f.close()
    
    def normalize_burstiness(self, cnt):
        if cnt<5:
            burstiness=0.05
        elif cnt<10:
            burstiness=0.2
        else:
            burstiness=0.75
        
        return burstiness
    
    def determine_burstiness(self, allReviews):
        #create list
        users =[]
        for r in allReviews:
            users.append(r['_id']['reviewerId'])
        
        cnt_dict=Counter(users)  
        
        for c in cnt_dict:
            try:
                self.B.node[c]['burstiness'] =  self.B.node[c]['burstiness'] + self.normalize_burstiness(cnt_dict[c])
            except:
                self.B.node[c]['burstiness'] =  self.float_precision(self.normalize_burstiness(cnt_dict[c]))
    
    def determine_content_similarity(self, allReviewers):
        
        for r in allReviewers:
            x=self.B.neighbors(str(r))
            
            if len(x)==1:
                self.B.node[r]['content_similarity'] = 0.0001
            else:
                cs=0.0001
                for i in range(0,len(x)-1):
                    rest1 = x[i]
                    rest2 = x[i+1]
                    text1 = self.B.edge[r][rest1]['reviewText']
                    text2 = self.B.edge[r][rest2]['reviewText']
                    cs=cs+get_cosine(text_to_vector(text1),text_to_vector(text2))
                    
                self.B.node[r]['content_similarity'] = self.float_precision(cs/len(x))
              
                    
    def get_bipartite_sets(self):
        restaurants_set = set(n for n,d in self.B.nodes(data=True) if d['bipartite']==0)
        self.reviewers_list = list(set(self.B) - restaurants_set)
        self.restaurants_list = list(restaurants_set)
        
    def normalize_rate_deviation(self, rd):
        return rd/10    
        
    def determine_rate_deviation(self, users_list):
        
        for u in users_list:
            r = self.B.neighbors(u)
            rd_sum = 0.001
            for i in range(0,len(r)):
                rest = r[i]
                deviation = (self.B.node[rest]['total_rating']/self.B.node[rest]['total_reviews'])-self.B.edge[rest][u]['rating']
                rd_sum = rd_sum+abs(deviation)
                
            self.B.node[u]['rate_deviation'] = self.float_precision(self.normalize_rate_deviation(rd_sum/len(r)))
             
    
    def get_spam_users_ts(self,allReviews_ts):
        for r in allReviews_ts:
            if r['type']=='spam':
                self.spam_users_ts.append(r['_id']['reviewerId'])
            
    
    
    def  __init__(self, *args, **kwargs):
    #First date in sorted database
        all_reviews = self.db_handle_sorted.find_one()
        start_date = all_reviews['reviewDate']
    
    #Setting the interval
        tmdelta = datetime.timedelta(weeks=4)
    
    #End date for the first timestamp
        end_date = start_date+tmdelta
        
    
    #Last date in the database
        final_end_date= datetime.datetime(2014,11,18,0,0,0)
    
    #Timestamp count
        cnt = 0
                
        while end_date <= final_end_date:
            cnt = cnt + 1 
            print "In timestamp: "+str(cnt)
            #Get reviews within current timestamp
            allReviews_ts = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}})    
            allReviewers_ts_distinct = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}}).distinct('_id.reviewerId')
            allRestaurants_ts_distinct = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}}).distinct('_id.restaurantId')
            
            self.B.add_nodes_from(allReviewers_ts_distinct, bipartite=1)
            self.B.add_nodes_from(allRestaurants_ts_distinct, bipartite=0)

            #Reviewer Feature 1 - Calculate the burstiness
            self.determine_burstiness(allReviews_ts)
            
            #Keep count of appearance of a user
            for u in allReviewers_ts_distinct:
                try:
                    self.B.node[u]['appearance']=self.B.node(u)['appearance']+1
                except:
                    self.B.node[u]['appearance']=1
            
            
            allReviews_ts = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}})  
            self.spam_users_ts = []
            self.get_spam_users_ts(allReviews_ts)              
            
            allReviews_ts = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}})          
            
            for i in range(0,allReviews_ts.count()):
                r = allReviews_ts[i]
                usr = str(r['_id']['reviewerId'])
                rest = r['_id']['restaurantId']
                
                #Check if restaurant is being reviewed for first time
                first_time = False
                try:
                    self.B.neighbors(rest)
                except:
                    first_time = True
                       
                self.B.add_edge(rest,usr)
                
                #Reviewer Feature 2 - First time count
                if first_time:    
                    try:
                        self.B.node[usr]['first_time'] = self.B.node(usr)['first_time']+1
                    except:
                        self.B.node[usr]['first_time'] = 1
                else:
                    self.B.node[usr]['first_time'] = 0.0001        
                        
                #Store properties of the reviews
                self.B[rest][usr]['rating'] = float(r['rating'])
                self.B[rest][usr]['reviewText'] = str(r['reviewText'])            
                
                try:
                    self.B.node[rest]['total_rating'] = self.B.node[rest]['total_rating'] + float(r['rating'])
                except:
                    self.B.node[rest]['total_rating']  = 1
                
                try:     
                    self.B.node[rest]['total_reviews'] = self.B.node[rest]['total_reviews']+1
                except:
                    self.B.node[rest]['total_reviews'] = 1
                
                try:     
                    self.B.node[usr]['total_reviews'] = self.B.node[usr]['total_reviews']+1
                except:
                    self.B.node[usr]['total_reviews'] = 1
                
                try:
                    if self.B.node[usr]['spamicity_score']==0.0001:     
                        self.B.node[usr]['spamicity_score'] = 0.0001
                except:
                    self.B.node[usr]['spamicity_score'] = 0.0001    
            
            #Reviewer Feature 3 - Content Similarity  
            self.determine_content_similarity(allReviewers_ts_distinct)
            
            #Reviewer Feature 4 - Rate Deviation
            affected_users = []
            for x in allRestaurants_ts_distinct:
                affected_users.extend(list(self.B.neighbors(x)))
            
            final_list = set(affected_users)
            final_list = list(final_list)
            self.determine_rate_deviation(final_list)
            
            
            
            print "Features Extracted"
            
            if (cnt==33):
                print "In ts 33: getting spamicity score" 
                dir_name = "snapshots/snapshot"+str(cnt)
                filenm = dir_name+str("/G_matrix.txt")
                lines = [line.strip() for line in open(filenm)]
                for user in lines:
                   rev = ast.literal_eval(user)
                   uid = rev['reviewerID']
                   self.B.node[uid]['spamicity_score']=rev['spam_score_total']                 
            
            if (cnt>=34):
                t0 = time()
                #Prediction
                allReviewers_ts_distinct = self.db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}}).distinct('_id.reviewerId')
                self.prediction(allReviewers_ts_distinct,cnt)
                t1=time();
                print "Prediction Done in : %f" % (t1 - t0), "secs.\n";
                #NMF - role determination
                t0 = time()
                self.apply_nmf(cnt)
                t1=time();
                print "NMF Done in : %f" % (t1 - t0), "secs.\n";
                if cnt == 130:
                    break
            
            #Goto next snapshot
            start_date = end_date
            end_date = end_date+tmdelta


if __name__ == "__main__":
    #main(sys.argv);
    x = SpamDetection('x','y')    

      
