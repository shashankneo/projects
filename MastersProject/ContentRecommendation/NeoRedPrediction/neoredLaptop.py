import util
from GoogleNews import GoogleNews
from RemoveBoiler import RemoveBoiler
from Relevance import Relevance
from Smoothness import Smoothness
from Clarity import ConnectionClarity
from NER import NER
from RecommendationMetric import RecommendationMetric
import traceback
  
    
def letsGoMates():    
    try:
        print 'a'
        util.initLogger()
        util.initGlobal()
        util.initSettings()
        #capture Google News & Its links
#         status = GoogleNews()
#         if status == False:
#             util.logger.error("Existing application because of GoogleNews not being updated for today")
#             return
#         else:
#             util.logger.info("Google news done for today")
        
        #Remove boiler data from Google News & Html
        status = RemoveBoiler()
        if status == False:
            util.logger.error("Existing application because of Boiler not being updated for today")
            return
        else:
            util.logger.info("Boiler done for today")
        
        #Do some relevance Test
        status = Relevance()
        if status == False:
            util.logger.error("Existing application because of Relevance not being updated for today")
            return
        else:
            util.logger.info("Relevance done for today")
        
        #Do some smoothness task
        status = Smoothness()
        if status == False:
            util.logger.error("Existing application because of Smoothness not being updated for today")
            return
        else:
            util.logger.info("Smoothness done for today")
        
        #Do some Connection Clarity check
        status = ConnectionClarity()
        if status == False:
            util.logger.error("Existing application because of ConnectionClarity not being updated for today")
            return
        else:
            util.logger.info("Connection clarity done for today")
            
        #Finally recommend something
        status = RecommendationMetric()
        if status == False:
            util.logger.error("Existing application recommendation for today")
            return
        else:
            util.logger.info("Recommended links done for today")
        #Finally write the recommendations to some folder
    #    sched = BackgroundScheduler()
    #    job = sched.add_job(downloadGoogleNewsSched, 'interval', seconds=2)
    #    sched.start()
    #    while True:
    #        time.sleep(10)
    #    sched.shutdown()
    except Exception, e:
        print "Exception at NeoRedPrediction : %s" % traceback.print_exc()
    
    
    
    
    
if __name__ == '__main__':
    letsGoMates()

   