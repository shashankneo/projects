from ConfigParser import SafeConfigParser
import Constants
import time
import logging
import os 
import datetime 
import re
import nltk
import string
import traceback
import json
import sys

packageName = "com.neoRed.prediction.settings"
config = None
logger = None
stopwords = None

def initLogger():
    global logger
    logger = logging.getLogger('neoRed')
    todayDateFolder = getTodayDateFolder()
    log_dir = os.path.join(Constants.ROOT_FOLDER,Constants.LOG_DIR,Constants.NEDREAD,todayDateFolder)
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    log_file = os.path.join(log_dir,Constants.LOG_FILE)
    if os.path.isfile(log_file) == False:
         file = open(log_file,'w')   # Trying to create a new file or open one
         file.close()
         
    hdlr = logging.FileHandler(log_file)
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    hdlr.setFormatter(formatter)
    logger.addHandler(hdlr) 
    logger.setLevel(logging.INFO)
def initStopWordsList():
    global stopwords
    stopwords = []
    f=open(os.path.join(Constants.ROOT_FOLDER,'stopwords.txt'),'r')
    fileText=f.read()
    for word in fileText.split('\n'):
        stopwords.append(word)
    f.close()
    f=open(os.path.join(Constants.ROOT_FOLDER,'hipwords.txt'),'r')
    fileText=f.read()
    for word in fileText.split('\n'):
        stopwords.append(word)
    f.close()
    f=open(os.path.join(Constants.ROOT_FOLDER,'uselesswords.txt'),'r')
    fileText=f.read()
    for word in fileText.split('\n'):
        stopwords.append(word)
    f.close()
    pass
def initGlobal():
    initStopWordsList()
    

def initSettings():
    global config
    config = SafeConfigParser()
    config.read('config.ini')
    if not config.has_section("main"):
        config.add_section("main")
    
    
def saveSettings(key, value):
    global config
    config.set('main', key, value)
    with open('config.ini', 'w') as f:
        config.write(f)  
def loadSettings(key):
    value = None
    if config.has_option("main",key):
       value = config.get("main",key)
    return value
        
    
def getTodayDateFolder():
    today = time.strftime("%d-%m-%Y")
    return today

def getDateFolder(date):
    day = date.strftime("%d-%m-%Y")
    return day

def writeToFile(data, filepath):
    result = False
    try:
        f = open(filepath, 'w')
        f.write(data)
        f.close()
        result = True
    except Exception, e:
            print "Exception at writing file: %s" % e
    return result

def readFromFile(filepath):
    data = None
    try:
        f = open(filepath,"r")
        data = f.read()
        f.close()   
    except Exception, e:
            print "Exception at writing file: %s" % e
    return data

def findTrainingDays():
    today = datetime.date.today()
    training = []
    delta = datetime.timedelta(days=1)
    for i in range(5):
        yesterday = today - delta
        yester_str = getDateFolder(yesterday)
        training.append(yester_str)
        today = yesterday
    return training

def findTrainingFiles():
    onlyfiles = []
    dates = findTrainingDays()
    for day in dates:
        folders = []
        folders.append(os.path.join(Constants.ROOT_FOLDER,Constants.BOILER_DATA_DIR,day))
        folders.append(os.path.join(Constants.ROOT_FOLDER,Constants.BOILER_DATA_DIR,day,Constants.FACEBOOK_DIR))
        folders.append(os.path.join(Constants.ROOT_FOLDER,Constants.BOILER_DATA_DIR,day, Constants.TWITTER_DIR))
        for files_dir in folders:
            if os.path.exists(files_dir):
                onlyfiles.extend([os.path.join(files_dir,
                     fi) for fi in os.listdir(files_dir) if os.path.isfile(os.path.join(files_dir,
                     fi))])
    return onlyfiles

def findTestFiles():
    onlyfiles = []
    day = getTodayDateFolder()
    folders = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.BOILER_GOOGLE_NEWS_DIR,day)
    if os.path.exists(folders):
        onlyfiles.extend([os.path.join(folders,
                     fi) for fi in os.listdir(folders) if os.path.isfile(os.path.join(folders,
                     fi))])
    return onlyfiles
def extract_text(filehtml):
    filetext = filehtml.encode('utf-8')
    filetext = re.sub(r'[^\x00-\x7F]+', ' ', filetext)
    filetext = filetext.lower()

    # remove the punctuation

    no_punctuation = filetext.translate(None, string.punctuation)
    filetext=re.sub("[^a-zA-Z]+", " ", no_punctuation)
    filetext=re.sub(r'\W*\b\w{1,3}\b', ' ', filetext)
    tokens = nltk.word_tokenize(filetext)
    filtered = [w for w in tokens if not w in stopwords]
    return filtered

def findCorpus(onlyfiles):
     default = 0
     corpus = []
     mapping = {}
     files = []
     for obj in onlyfiles:
         filehtml = open(obj, 'r')
         try:
             head, tail = os.path.split(obj)
             if mapping.has_key(tail) is False and tail != '.DS_Store':
                 text = filehtml.read()
                 text = extract_text(text)
                 corpus.append(text)
                 mapping[tail] = default
                 files.append(tail) 
         except Exception:
             print traceback.format_exc()
         finally:
            if filehtml:
                filehtml.close()
     return corpus, files
 

     