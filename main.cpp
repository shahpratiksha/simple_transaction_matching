#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;

struct orderInfo
{
    //store the order number for the current order
    int order_id;
    //market, limit, stop, cancel
    std::string order_type;
    //true= buy or false=sell
    std::string action;
    // of shares, note could be updated in case of partial fill
    int number;
    //could be exact or threshold
    double price; 
    bool cancelled;
    //only valid for stop orders
    bool triggered;
    
    orderInfo(int id, std::string type, std::string act,
              int no, double pr, bool cancelled=false, bool triggered=false):
                order_id(id), order_type(type), action(act), number(no), price(pr) {}
};

void get_orders(std::vector<orderInfo>& orders)
{
    std::string new_order;
    int order_id=1;
    char* tokens[5];
    
    while(true) {
        
        getline(cin, new_order);
        char* order = new char[new_order.length()+1];
        strcpy(order,new_order.c_str());
        tokens[4] = strtok(order, " ");
        int part=0;
        
        while(tokens!=NULL) {
            tokens[4]=strtok(NULL, " ");
            tokens[part] = tokens[4];
            part++;
        }
        
        orders.push_back(orderInfo(order_id, std::string(tokens[0]), std::string(tokens[1]),atoi(tokens[2]),atof(tokens[3]),false,false));
        
        order_id++;
    }
}

/*if already cancelled, already executed, non-existent then no-op
if partially filled, cancel remaining number*/
//TODO: should this return bool?
bool cancel_order(orderInfo& order)
{
    /*this would indicate non-existent or filled order*/
   // if (order == NULL) -> use find here
     //   return true;
    if(order.cancelled == true)
        return true;
    //The moment we fill an order, we remove it from our list
    //if(unlikely(order.number==0)) // assuming gcc compiler
      if(order.number==0)  
        return true;
    
    order.cancelled=true;
    return true;
}

/*trigger any stop orders*/
void trigger_stop_orders(std::vector<orderInfo>&orders, double execute_price) {
        
    for(std::vector<orderInfo>::iterator itr = orders.begin(); itr < orders.end(); itr++) {
        if (itr->order_type == "stop"){
            if (itr->action == "sell" && execute_price <= itr->price)
                itr->triggered = true;
            
            if (itr->action == "buy" && execute_price >= itr->price)
                itr->triggered = true;
                
        }
     // TODO: now that we've triggered the relevant stop orders, see if any of them can be filled

    }
}

void execute_triggered_stop_orders(){
    //start from the earliest triggered stop, create candidates,
   //call pick_winner,etc
}

//TODO: this function should also erase any completed trades
void print_trade_success(orderInfo& taker, orderInfo& maker, std::vector<orderInfo>& orders){
    //TODO: make this smarter
    int remaining, executed;
    if(taker.number < maker.number) {
        remaining = maker.number - taker.number;
        maker.number = remaining;
        executed = taker.number;
        taker.number = 0;
        //TODO: do find/erase instead orders.erase(taker);
    }
    else {
        remaining = taker.number - maker.number;
        taker.number = remaining;
        executed = maker.number;
        maker.number=0;
        //TODO: do find/erase instead orders.erase(maker);
    }
       
    //TODO: price needs some thought
    cout << "match " << taker.order_id << " " << maker.order_id << " " << executed << " " << taker.price;    
    trigger_stop_orders(orders, taker.price/*TODO*/);
    execute_triggered_stop_orders();
}

//TODO:doesn't need to be a separate function
void add_to_candidate(orderInfo& order, std::vector<orderInfo> candidate_orders)
{
    candidate_orders.push_back(order);
}

void pick_winner(std::vector<orderInfo>candidate_orders, orderInfo& target_order)
{
    //either earliest, or cheapest if buy, and best offer if sell. match multiple if partial order fill, update order to reflect amount filled
    //need price and action from target_order
    //if stop order, make sure its triggered
    //make sure target_order.number is non-zero
    //print_trade_success
}

/* if buy order, look for lowest limit sell or any triggered stop order upto that point and vice versa
each filled order might trigger a stop order transaction-> TODO: think partial fill limit triggers stop
and that takes preference over anything else*/
void execute_market_order(orderInfo& target_order, std::vector<orderInfo>& orders){
    std::vector<orderInfo> stop_candidates,limit_candidates;
    // if sell, look for corresponding buy limit or triggered stop buy
    for(std::vector<orderInfo>::iterator itr = orders.begin(); itr->order_id < target_order.order_id; ++itr){
        
        if ((target_order.action == "sell" && itr->action == "buy") || (target_order.action == "buy" && itr->action == "sell")) {
            // find all candidates
            if (itr->order_type == "stop" && itr->triggered == true && itr->cancelled !=true){
                add_to_candidate(*itr, stop_candidates);
            }   
         
            if (itr->order_type == "limit" && itr->cancelled != true)
                add_to_candidate(*itr, limit_candidates);
        }
    }
    
        pick_winner(stop_candidates, target_order);
        pick_winner(limit_candidates, target_order);
}
    
//void try_match_order(std::string taker_action, std::string maker_action, std::vector<orderInfo>& orders){
    
//}



//Look at all orders before your order_id to find a match. and obviously look for sell-buy pairs
void try_execute_limit_order(orderInfo& order,std::vector<orderInfo>&orders)
{
    //std::string desired_type = order.action;
    std::vector<orderInfo> stop_candidates,limit_candidates;
    for(std::vector<orderInfo>::iterator itr = orders.begin(); itr->order_id < order.order_id; itr++) {
        // note this is valid because we're taking care of (and removing) all cancel orders immediately
        //if(unlikely(itr->action == "none"))
        if(itr->action == "none")
            continue;
        //TODO: use candidate matching here
        if(itr->action == "sell" && order.action == "buy" && itr->price <= order.price)add_to_candidate(*itr, stop_candidates);
            add_to_candidate(*itr, itr->order_type == "stop" ? stop_candidates : limit_candidates);
        if(itr->action == "buy" && order.action == "sell" && itr->price >= order.price)
            add_to_candidate(*itr,itr->order_type == "stop" ? stop_candidates : limit_candidates);
     }
    pick_winner(stop_candidates, order);
    pick_winner(limit_candidates, order);
}

void match_orders(std::vector<orderInfo>& orders) {
    // iterate through all orders to start taking action.
    for(std::vector<orderInfo>::iterator itr = orders.begin(); itr < orders.end(); itr++) {
        
        if(itr->cancelled == true || itr->number == 0 )
            orders.erase(itr);
        
        if(itr->order_type == "cancel")
           if(cancel_order(*itr))
              orders.erase(itr);
        
        if(itr->order_type == "market"){
           execute_market_order(*itr, orders);
           orders.erase(itr); // any unfilled market order is dropped
        //iterate through remaining orders to find the first buy and first sell    
        }
        
        if(itr->order_type == "limit"){
            try_execute_limit_order(*itr,orders);
        }
        
        if(itr->order_type == "stop"){
            printf("this is a stop order, will wait to trigger");
        }
    }
}
           
int main() {  
    std::vector<orderInfo> inputOrders; /*this initially holds all the input orders, and stuff gets popped out each time a trade is made*/
    get_orders(inputOrders);
    match_orders(inputOrders);
    return 0;
}
