import os
import Constants
import traceback
import util
import json
from collections import Counter
import operator

def readLinksJson(downloadDate):
    jsonData = None
    try:
        readDirectory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.URL_DIR
                                         ,downloadDate,Constants.RECOMMINFO_FILE)
        if os.path.isfile(readDirectory) is True:
             with open(readDirectory) as json_data:
                json_text = json_data.read()
                jsonData = json.loads(json_text)
                json_data.close()
    except Exception, e:
        util.logger.error("Exception at read Boiler recomm json for : %s" % downloadDate)
    return jsonData

def printRecommendedDocs(recDocs, downloadDate):
    jsonData = readLinksJson(downloadDate)
    if jsonData is None:
        return False
    
    result = False
    final_json = {}
    algo1List = []
    try:
        sorted_x = sorted(recDocs.items(), key=operator.itemgetter(1))
        todayDateFolder = util.getTodayDateFolder()
        write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.FINAL_DIR,todayDateFolder)
        if not os.path.exists(write_directory):
                os.makedirs(write_directory)
        outfile = open(os.path.join(write_directory,Constants.ULTIMATE_FILE), 'w')
        json_write = {}
        count = 0
        for (key,val) in sorted_x:
            if key not in jsonData:
                continue
            linkObj = {}
            linkObj['url'] = jsonData[key]['url']
            linkObj['content'] = jsonData[key]['content']
            linkObj['title'] = jsonData[key]['title']
            linkObj['id'] = key
            linkObj['use']  = 'NA'
            algo1List.append(linkObj)
            count = count + 1
            if count >= Constants.RECOMMENDED_LINKS:
                break
        final_json['algo1'] = algo1List
        json.dump(final_json, outfile)
        outfile.close()
        result = True
    except Exception, e:
        util.logger.error("Exception at writing final Recommendation docs for data : %s" % write_directory)
    return result

def RecommendationMetric():
     todayDateFolder = util.getTodayDateFolder()
     lastRecommended= util.loadSettings(Constants.LAST_RECOMMENDATION_DONE)
     if todayDateFolder == lastRecommended:
         return True
     try:
         relevance_json = {}
                  
         #Get Relevance json
         readRelevanceDir = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.ENGINE_DIR
                                         ,todayDateFolder,Constants.RELEVANCE_FILE)
         if os.path.isfile(readRelevanceDir) is True:
             with open(readRelevanceDir) as json_data:
                json_text = json_data.read()
                relevance_json = json.loads(json_text)
                json_data.close()
            
        #Get Smoothness json
         smoothness_json = {}
         readSmoothnessDir = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.ENGINE_DIR
                                         ,todayDateFolder,Constants.SMOOTHNESS_FILE)
         if os.path.isfile(readSmoothnessDir) is True:
             with open(readSmoothnessDir) as json_data:
                json_text = json_data.read()
                smoothness_json = json.loads(json_text)
                json_data.close()
            
        #Get Clarity json
         clarity_json = {}
         
         readClarityDir = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.ENGINE_DIR
                                         ,todayDateFolder,Constants.CLARITY_FILE)
         if os.path.isfile(readClarityDir) is True:
             with open(readClarityDir) as json_data:
                json_text = json_data.read()
                clarity_json = json.loads(json_text)
                json_data.close()
                
        #Lets not apply any linear weight formula for now
         cou = Counter()
         cou.update(relevance_json)
         cou.update(smoothness_json)
         cou.update(clarity_json)
         
         #Convert above back to a dictionary
         final_json = dict(cou)
         result = printRecommendedDocs(final_json, todayDateFolder)
         if result == True:
             util.saveSettings(Constants.LAST_RECOMMENDATION_DONE, todayDateFolder)
             util.logger.info("Recommended links done for ="+ todayDateFolder)
         pass
     except Exception, e:
            util.logger.error( "Exception at recommending links for : %s Exception = %s" 
                               % (todayDateFolder,traceback.print_exc))
     pass