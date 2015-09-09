

from selenium import webdriver
import os
import Constants
import util
import re
import urllib2
from bs4 import BeautifulSoup
import json
downloadedLinks = []

def writeUrlJson(jsonData, downloadDate):
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
        util.logger.error("Exception at writing Google news  link json for : %s" % downloadDate)
    return result
def downloadGoogleNews(downloadDate):
    result = False
    read_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.GOOGLE_LINKS_DIR,downloadDate,
                                  Constants.GOOGLE_LINKS_FILE)
    write_directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.GOOGLE_NEWS_DIR,downloadDate)
    if not os.path.exists(write_directory):
            os.makedirs(write_directory)
    writeJson = {}
    try:
        hyperlinks = [line.strip() for line in open(read_directory)]
        count = 0
        for link in hyperlinks:
             url=link.replace("http://", "")
             url=url.replace("www.", "")
             parsedUrl=re.sub(r'\W+', '', url)
             if len(parsedUrl) > 25:
             parsedUrl=parsedUrl[:25]
             try:
                 
                 html_filename = os.path.join(write_directory,parsedUrl)
                 if os.path.isfile(html_filename) is False:
                      htmlfile = urllib2.urlopen(link)
                      html = htmlfile.read()
                      ret = util.writeToFile(html,html_filename)
                
                      if ret == True:
                          linkDict = {}
                          linkDict["url"] = link
                          linkDict["content"] = ""
                          soup = BeautifulSoup(html, 'html.parser')
                          if soup.title and soup.title.contents[0]:
                              title = soup.title.contents[0]
                          else:
                              title = ""
                          linkDict["title"] = title
                          writeJson[parsedUrl] = linkDict
                 count=count+1
                 print 'downloaded link ='+url
             except Exception, e:
                 util.logger.error("Exception at downloading link : %s" % url)
             
             
        
        if count > Constants.MIN_GOOGLELINKS_DAILY:
            result = writeUrlJson(writeJson, downloadDate )
            if result == True:
            util.saveSettings(Constants.LAST_GOOGLENEWS_DOWNLOAD, downloadDate)
            util.logger.info("Google news downloaded for ="+downloadDate+" links="+str(count))
            
        else:
            util.logger.error("Google news failed to download for ="+downloadDate+" links="+str(count))
    except Exception, e:
        print "Exception at open Google news links for download: %s" % read_directory
    return result

def getLinksPerCategory(links, category):
    count = 0
    for link in links:
        try:
            url = str(link.get_attribute('url'))
            if url not in downloadedLinks:
                downloadedLinks.append(url)
                count = count + 1
        except Exception, e:
            util.logger.error("Exception at retrieving links per Google category: %s" % link)
    util.logger.info(category + 'Google news page links total= ' + str(count))

def getGoogleLinks(todayDate):
    categories = {'Technology':"http://news.google.com/news/section?pz=1&cf=all&topic=tc",
                  'Entertainment':"http://news.google.com/news/section?cf=all&pz=1&topic=e",
                  'Sports':"http://news.google.com/news/section?pz=1&cf=all&topic=s",
                  'Science':"http://news.google.com/news/section?pz=1&cf=all&topic=snc",
                  'Health':"http://news.google.com/news/section?pz=1&cf=all&topic=m",
                  'More Top Stories':"http://news.google.com/news/section?pz=1&cf=all&topic=h",
                  'Spotlight':"http://news.google.com/news/section?pz=1&cf=all&topic=ir",
                  'Location':"http://news.google.com/news/section?cf=all&pz=1&geo=detect_metro_area"}
    result = False
    
    #Open the google.com initially
    count_browsers = 0
    while True:
        try:
            if count_browsers == 0:
               browser = webdriver.Chrome() 
            elif count_browsers ==1:
               browser = webdriver.Firefox()
            else:
                browser = webdriver.Safari()    
            break
        except Exception, e:
            count_browsers = count_browsers + 1
    if count_browsers == 3:
        util.logger.error("CODE RED: No browser driver present for selenium Google News download")
        return result
    
    browser.implicitly_wait(10)
    browser.get('http://www.news.google.com')
    moreCategories = browser.find_elements_by_xpath('//a[@class="persistentblue"]')
    
    # Get links of remaining categories
    for category in moreCategories:
        url = str(category.get_attribute('href'))
        text = category.text
        text=text.encode('utf-8')
        #If this category is already included. Then move to next category
        if text in categories:
            continue
        categories[text] = url
        
    for category in categories.keys():
        browser.get(categories[category])
        links = browser.find_elements_by_xpath('//a')
        getLinksPerCategory(links, category)
        

    # Get all the main page links.
    #Initialize for it
    browser.get('http://www.news.google.com')
    links = browser.find_elements_by_xpath('//a')
    getLinksPerCategory(links, 'HomePage')
    
    if len(downloadedLinks) > Constants.MIN_GOOGLELINKS_DAILY:
        linksToBeWritten = "\n".join(downloadedLinks)
        directory = os.path.join(Constants.ROOT_FOLDER,Constants.RECOMMENDATION_DIR,Constants.GOOGLE_LINKS_DIR,todayDate)
        if not os.path.exists(directory):
            os.makedirs(directory)
        result = util.writeToFile(linksToBeWritten,os.path.join(directory,Constants.GOOGLE_LINKS_FILE))
        if result == True:
            util.saveSettings(Constants.LAST_GOOGLELINKS_DOWNLOAD, todayDate)
            util.logger.info("Google links downloaded for ="+todayDate)
            return result
            
    util.logger.error("Google links not downloaded for ="+todayDate)
    return result        
            
def GoogleNews():
    downloadedLinks = []
    todayDate = util.getTodayDateFolder()
    lastNewsDownloaded = util.loadSettings(Constants.LAST_GOOGLENEWS_DOWNLOAD)
    lastLinksDownloaded = util.loadSettings(Constants.LAST_GOOGLELINKS_DOWNLOAD)
    
    googleLinksStatus = True
    googleNewsStatus = True
    #Check whether today links have been extracted or not
    if lastLinksDownloaded != todayDate:
        googleLinksStatus = getGoogleLinks(todayDate)
    else:
        util.logger.info("Google links downloaded successfully for = "+todayDate)
                     
    #Check whether today news has been extracted or not
    if todayDate != lastNewsDownloaded:
      googleNewsStatus =  downloadGoogleNews(todayDate)
    else:
        util.logger.info("Google news already downloaded successfully for = "+ todayDate)
    
    return googleLinksStatus & googleNewsStatus
    