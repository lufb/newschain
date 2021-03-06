//
// Created by boy on 18-6-25.
//

#pragma once

#include <news/chain/global_property_object.hpp>
#include <news/plugins/json_rpc/utility.hpp>

#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>

#include <news/base/account_object.hpp>
#include <news/base/asset.hpp>
#include <news/chain/transaction.hpp>

using namespace news::chain;
using namespace news::plugins::json_rpc;




namespace news{
    namespace plugins{
        namespace database_api{


            /*
             * dynamic_global_property
             * */
            typedef void_type                       get_dynamic_global_property_args;
            typedef dynamic_global_property_object  get_dynamic_global_property_return;


            struct account{
                account(){}
                account(const news::base::account_object &obj)
                :name(obj.name)
                        ,creator(obj.creator)
                        , balance(obj.balance)
                        , create_time(obj.create_time){

                }
                account_name                name;
                account_name                creator;
                news::base::asset           balance;
                fc::time_point              create_time;
            };



            struct get_accounts_args{
                std::set<account_name> accounts;
            };


            struct get_accounts_return{
                std::vector<account>    accounts;
            };


            struct get_transactions_hex_args{
                news::chain::signed_transaction trx;
            };

            struct get_transactions_hex_return{
//                get_transactions_hex_return(std::string &_hash):hash(_hash){}
                std::string hash;
            };


            struct get_transaction_args{
                news::base::transaction_id_type     id;
            };

            typedef news::chain::deatil_signed_transaction  get_transaction_return;



        }
    }
}

FC_REFLECT(news::plugins::database_api::account, (name)(creator)(balance)(create_time))
FC_REFLECT(news::plugins::database_api::get_accounts_args, (accounts))
FC_REFLECT(news::plugins::database_api::get_accounts_return, (accounts))
FC_REFLECT(news::plugins::database_api::get_transactions_hex_args, (trx))
FC_REFLECT(news::plugins::database_api::get_transactions_hex_return, (hash))
FC_REFLECT(news::plugins::database_api::get_transaction_args, (id))
