#!/usr/bin/python
# -*- coding: utf-8 -*-


from gensim import corpora
import operator
import util
import Constants
import os
import json
import math
from numpy import zeros
import numpy as np

def printRankedDocs(smoothness, usedTestFiles):
    result = False
    try:
    testScore = smoothness.sum(axis=1)
    testMapping = {}
    for files in range(len(usedTestFiles)):
        testMapping[usedTestFiles[files]] = testScore[files] 
    sorted_x = sorted(testMapping.items(), key=operator.itemgetter(1), reverse=True)
    todayDateFolder = util.getTodayDateFolder()
    write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.ENGINE_DIR,todayDateFolder)
    if not os.path.exists(write_directory):
            os.makedirs(write_directory)
    outfile = open(os.path.join(write_directory,Constants.SMOOTHNESS_FILE), 'w')
    json_write = {}
    count = 1
    for (key,val) in sorted_x:
        json_write[key] = count
        count = count + 1
    json.dump(json_write, outfile)
    outfile.close()
        result = True
    except Exception, e:
        util.logger.eror("Exception at printing Smoothness docs for data : %s" % write_directory)
    return result

def Smoothness():
    todayDate = util.getTodayDateFolder()
    lastSmoothnessDate = util.loadSettings(Constants.LAST_SMOOTHNESS_DIR)
    if todayDate == lastSmoothnessDate :
        util.logger.info("Smoothness signal done for today" + todayDate)
        return True
    trainFiles = util.findTrainingFiles()
    testFiles = util.findTestFiles()
    trainCorpus, usedTrainFiles = util.findCorpus(trainFiles)
    testCorpus, usedTestFiles = util.findCorpus(testFiles)   
    bm25obj = Bm25(trainCorpus)
    testJson = {}
    testUniqueWords = []
    trainUniqueWords = []
    smoothness=zeros((len(testCorpus),len(trainCorpus)))
    for testText in testCorpus:
        testUniqueWords.append(set(testText))
    for trainText in trainCorpus:
        trainUniqueWords.append(set(trainText))
    
    for testDoc in range(len(testCorpus)):
        uniqueTest = testUniqueWords[testDoc]
        SminusDcontext=zeros(bm25obj.N)
        DminusScontext=zeros(bm25obj.N)
        for trainDoc in range(len(trainCorpus)):
            uniqueTrain = trainUniqueWords[trainDoc]
            SminusD = [word for word in trainCorpus[trainDoc] if word not in uniqueTest]
            DminusS = [word for word in testCorpus[testDoc] if word not in uniqueTrain]
            SminusDcontext = bm25obj.BM25Score(SminusD)
            DminusScontext = bm25obj.BM25Score(DminusS)
            smoothness[testDoc][trainDoc]=np.dot(SminusDcontext,DminusScontext)
    ret = printRankedDocs(smoothness, usedTestFiles)
    if ret == True:
        util.saveSettings(Constants.LAST_SMOOTHNESS_DIR, todayDate)
        util.logger.info("Smoothness info just completed for ="+todayDate)
    return ret

class Bm25:
    
    def __init__(self, fn_docs) :
        self.fn_docs = fn_docs
        self.N = len(fn_docs)
        self.DocAvgLen = 0
        self.DF = {}
        self.DocTF = []
        self.DocLen = []
        self.DocIDF = {}
        self.buildDictionary()
        self.TFIDF_Generator()
        
        
    def buildDictionary(self) :
        docs = self.fn_docs
        all_tokens = sum(docs, [])
        tokens_once = set(word for word in set(all_tokens) if all_tokens.count(word) == 1)
        TotWords = len(all_tokens) - len(tokens_once)
        self.DocAvgLen = TotWords/self.N
        self.trainCorpus = [[word for word in text if word not in tokens_once]
         for text in docs]
        self.dictionary = corpora.Dictionary(self.trainCorpus)
        
    def TFIDF_Generator(self, base=math.e) :
        trainCorpus = self.trainCorpus
        for doc in trainCorpus:
            self.DocLen.append(len(doc))
            bow = dict([(term, freq*1.0/len(doc)) for term, freq in self.dictionary.doc2bow(doc)])
            for term, tf in bow.items() :
                    if term not in self.DF :
                        self.DF[term] = 0
                    self.DF[term] += 1
            self.DocTF.append(bow)
                
        for term in self.DF:
            self.DocIDF[term] = math.log((self.N - self.DF[term] +0.5) / (self.DF[term] + 0.5), base)
    
    def BM25Score(self, Query=[], k1=1.5, b=0.75) :
        query_bow = self.dictionary.doc2bow(Query)
        scores = []
        for idx, doc in enumerate(self.DocTF) :
            commonTerms = set(dict(query_bow).keys()) & set(doc.keys())
            tmp_score = []
            doc_terms_len = self.DocLen[idx]
            for term in commonTerms :
                upper = (doc[term] * (k1+1))
                below = ((doc[term]) + k1*(1 - b + b*doc_terms_len/self.DocAvgLen))
                tmp_score.append(self.DocIDF[term] * upper / below)
            scores.append(sum(tmp_score))
        return np.array(scores)

            