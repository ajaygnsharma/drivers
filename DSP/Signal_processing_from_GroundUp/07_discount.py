
price=37692.0
discount=[0,5,10,15,20,25,30,35,38,40];
final_price=[];

for i in discount:
    p = (price * (100 - i))/100;
    final_price.append(p);

print(final_price);
