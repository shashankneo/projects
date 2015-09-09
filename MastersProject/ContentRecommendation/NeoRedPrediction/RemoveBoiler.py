import util
import Constants
import os
import json

from boilerpipe.extract import Extractor

def writeBoilerJson(jsonData, downloadDate):
    result = False
    try:
        write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,
                                       Constants.URL_DIR,downloadDate)
        if not os.path.exists(write_directory):
                    os.makedirs(write_directory)
        outfile = open(os.path.join(write_directory,Constants.RECOMMINFO_FILE), 'w')
        json.dump(jsonData, outfile)
        outfile.close()
        result = True   
    except Exception, e:
        util.logger.error("Exception at writing Boiler recomm json for : %s" % downloadDate)
    return result

def readBoilerJson(downloadDate):
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

def remove_boiler(htmlD):
    extractor = Extractor(extractor='DefaultExtractor', html=htmlD)
    text=extractor.getText().encode('ascii', 'ignore').decode('ascii')
    return text

def BoilerNews(downloadDate):
    jsonData = readBoilerJson(downloadDate)
    if jsonData is None:
        return False
    result = False
    read_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.GOOGLE_NEWS_DIR,downloadDate)
    write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.BOILER_GOOGLE_NEWS_DIR,downloadDate)
    if not os.path.exists(read_directory):
        util.logger.error("Boilers news can't be run because folder isn't present = "+downloadDate)
        return result
    if not os.path.exists(write_directory):
            os.makedirs(write_directory)
    
    onlyfiles = [ f for f in os.listdir(read_directory) if os.path.isfile(os.path.join(read_directory,f)) ]  
    count = 0      
    for htmlFile in onlyfiles:    
        try:
            htmlData = util.readFromFile(os.path.join(read_directory,htmlFile))      
            if htmlData is not None:
                
                html_filename = os.path.join(write_directory,htmlFile)
                if os.path.isfile(html_filename) is False:
                    htmlText = remove_boiler(htmlData)
                    result = util.writeToFile(htmlText, html_filename) 
                    if result == True:
                        if htmlFile in jsonData:
                            jsonData[htmlFile]["content"] = htmlText
                else:
                    result = True
                if result == True:
                    count = count + 1
                else:
                    if htmlFile in jsonData:
                        del jsonData[htmlFile]
                print 'Boilered done for ='+html_filename+str(count)
        except Exception, e:
            util.logger.error( "Exception at boiler for google news : %s" % read_directory)
    if ((count*100)/len(onlyfiles)) > Constants.MIN_GOOGLELINKS_DAILY:
        result = writeBoilerJson(jsonData, downloadDate )
        if result == True:
        util.saveSettings(Constants.LAST_BOILER_GOOGLENEWS, downloadDate)
        util.logger.info("Google news boilered for ="+downloadDate+" links="+str(count))
        
    else:
        util.logger.error("Google news failed to boilered for ="+downloadDate+" links="+str(count))
    return result
        
def BoilerData(downloadDate):
    ret = False
    read_directory = os.path.join(Constants.ROOT_FOLDER,Constants.DATA_DIR,downloadDate)
    write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.BOILER_DATA_DIR,downloadDate)
    if not os.path.exists(read_directory):
        util.logger.error("Boilers data can't be run because folder isn't present = "+downloadDate)
        return ret
    if not os.path.exists(write_directory):
            os.makedirs(write_directory)
    
    onlyfiles = [ f for f in os.listdir(read_directory) if os.path.isfile(os.path.join(read_directory,f)) ]  
    count = 0      
    try:
        for htmlFile in onlyfiles:
            htmlData = util.readFromFile(os.path.join(read_directory,htmlFile))
            html_filename = os.path.join(write_directory,htmlFile)
            if os.path.isfile(html_filename) is False:
                htmlText = remove_boiler(htmlData)
                result = util.writeToFile(htmlText, html_filename) 
            else:
                result = True
            if result == True:
                count = count + 1
            util.logger.info('Boilered data done for ='+html_filename+str(count))
    except Exception, e:
        util.logger.error("Exception at boiler for data : %s" % read_directory)
    if ((count*100)/len(onlyfiles)) > Constants.MIN_GOOGLELINKS_DAILY:
        util.saveSettings(Constants.LAST_BOILER_DATA_DIR, downloadDate)
        util.logger.info("datadir boilered for ="+downloadDate+" links="+str(count))
        ret = True
    else:
        util.logger.error("datadir failed to boilered for ="+downloadDate+" links="+str(count))
    return ret
        
def RemoveBoiler():
    
     todayDate = util.getTodayDateFolder()
     lastNewsBoiled = util.loadSettings(Constants.LAST_BOILER_GOOGLENEWS)
     lastDataBoiled = util.loadSettings(Constants.LAST_BOILER_DATA_DIR)
     
     boilerNewsStatus = True
     boilerDataStatus = True
     
     #Check whether today links have been extracted or not
#      if lastNewsBoiled != todayDate:
#         boilerNewsStatus = BoilerNews(todayDate)
#      else:
#          util.logger.info("Boiler news already done for today :" + todayDate)
#                      
     trainingFolders = util.findTrainingDays()  
     anyTrainingFolderBoiled = True
     if lastDataBoiled == trainingFolders[0]:
         util.logger.info("Boiler data already done for today :" + lastDataBoiled)
     else:
         anyTrainingFolderBoiled = False
         folderIndex = 0
         if lastDataBoiled != None:
             folderIndex = trainingFolders.index(lastDataBoiled) 
         else:
             folderIndex = Constants.TRAINING_DAY
         if folderIndex < 0:
             folderIndex = 0
             util.logger.info("Boiler data for none of the last %d days have been downloaded" % Constants.TRAINING_DAY)
         for folder in range(folderIndex - 1, -1, -1):                   
                boilerDataStatus = BoilerData(trainingFolders[folder])
                if boilerDataStatus == False:
                    util.logger.error("Boiler data not done for today :" + trainingFolders[folder])
                else:
                    anyTrainingFolderBoiled = True
    
     return boilerNewsStatus & anyTrainingFolderBoiled
    
    