import logging
from apscheduler.schedulers.background import BackgroundScheduler
import util
from GoogleNews import GoogleNews
from RemoveBoiler import RemoveBoiler
from Relevance import Relevance
from Smoothness import Smoothness
from Clarity import ConnectionClarity
from NER import NER
def downloadGoogleNewsSched():
    GoogleNews()
  
    
    
    
    
    
    
    
    
if __name__ == '__main__':
    util.initLogger()
    util.initGlobal()
    util.initSettings()
    ConnectionClarity()
   # downloadGoogleNewsSched()
   # RemoveBoiler()
#    sched = BackgroundScheduler()
#    job = sched.add_job(downloadGoogleNewsSched, 'interval', seconds=2)
#    sched.start()
#    while True:
#        time.sleep(10)
#    sched.shutdown()
   