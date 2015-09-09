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
import nltk
def printRankedDocs(smoothness, usedTestFiles):
    
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


def NER():
    trainFiles = util.findTrainingFiles()
    testFiles = util.findTestFiles()
    trainCorpus, usedTrainFiles = util.findCorpus(trainFiles)
    testCorpus, usedTestFiles = util.findCorpus(testFiles)   
    for text in trainCorpus:
        nerText = "Chelsea began their Premier League title defence with a thrilling 2-2 draw against Swansea, as Thibaut Courtois was sent off and will now miss the crunch clash with Manchester City"
        for sent in nltk.sent_tokenize(nerText):
            for chunk in nltk.ne_chunk(nltk.pos_tag(nltk.word_tokenize(sent))):
                 if hasattr(chunk, 'node'):
                     print chunk.node, ' '.join(c[0] for c in chunk.leaves())