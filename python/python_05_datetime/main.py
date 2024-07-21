import datetime

expire_date = datetime.datetime.now()
#expire_date = expire_date + datetime.timedelta(days=90)
expire_date -= datetime.timedelta(days=1)

print(expire_date);
