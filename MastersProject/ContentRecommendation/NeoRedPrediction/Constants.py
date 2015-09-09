import sys
import os

def root_path():
    path = sys.executable
    listPath = path.split("/")
    listPath = listPath[:-4]
    return "/".join(listPath)


NEDREAD = "NeoRedPrediction"
MIN_GOOGLELINKS_DAILY = 20
#ROOT_FOLDER = "/Users/neocfc/Documents/workspace/ClientSideNeoRed"
ROOT_FOLDER = root_path()
RECOMMENDATION_DIR ="Recommendation_Dir"
LINKS_DIR ="linksDir"
LOG_DIR = "Log"
DATA_DIR = "dataDir"
FACEBOOK_DIR = "facebookDir"
TWITTER_DIR = "twitterDir"
ENGINE_DIR = "engineDir"
RELEVANCE_FILE = "relevance.json"
SMOOTHNESS_FILE = "smoothness.json"
CLARITY_FILE = "clarity.json"
RECOMMINFO_FILE = "recomminfo.json"
GOOGLE_LINKS_DIR ="Google_Links_Dir"
GOOGLE_NEWS_DIR ="Google_News_Dir"
GOOGLE_LINKS_FILE = "Google_Links_File"
BOILER_GOOGLE_NEWS_DIR = "Boiler_Google_News_Dir"
BOILER_DATA_DIR = "Boiler_Data_Dir"
FINAL_DIR = "Final_dir"
URL_DIR = "Url_dir"
ULTIMATE_FILE = "ultimate.json"
LOG_FILE = "prediction_log.txt"
LAST_GOOGLENEWS_DOWNLOAD = "LAST_GOOGLENEWS_DOWNLOAD"
LAST_GOOGLELINKS_DOWNLOAD = "LAST_GOOGLELINKS_DOWNLOAD"
LAST_BOILER_GOOGLENEWS = "LAST_BOILER_GOOGLENEWS"
LAST_BOILER_DATA_DIR = "LAST_BOILER_DATA_DIR"
LAST_RELEVANCE_DIR = "LAST_RELEVANCE_DIR"
LAST_SMOOTHNESS_DIR = "LAST_SMOOTHNESS_DIR"
LAST_CLARITY_DIR = "LAST_CLARITY_DIR"
LAST_RECOMMENDATION_DONE = "LAST_RECOMMENDATION_DONE"
TRAINING_DAY = 5
RECOMMENDED_LINKS = 40

