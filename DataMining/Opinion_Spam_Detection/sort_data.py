import pymongo
import datetime
import ast
import networkx as nx

con = pymongo.Connection()
db = con.dm_proj
db_handle = db.reviews
db_handle_sorted = db.sorted_reviews

    
all_reviews = db_handle_sorted.find()
current_ptr=0
start_date = all_reviews[0]['reviewDate']
tmdelta = datetime.timedelta(weeks=4)
end_date = start_date+tmdelta
final_end_date= datetime.datetime(2014,11,18,0,0,0)

while end_date <= final_end_date:
    allReviews_ts = db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}})
    allReviewers_ts = db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}}).distinct('_id.reviewerId')
    allRestaurants_ts = db_handle_sorted.find({"reviewDate": {"$gte": start_date, "$lt": end_date}}).distinct('_id.restaurantId')
  
    start_date = end_date
    end_date = end_date+tmdelta
    
