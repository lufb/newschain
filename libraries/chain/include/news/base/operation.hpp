//
// Created by boy on 18-6-18.
//

#pragma once

#include <news/base/types.hpp>
#include <news/base/asset.hpp>
#include <news/base/config.hpp>
#include <news/base/authority.hpp>

#include <fc/static_variant.hpp>
#include <fc/reflect/reflect.hpp>
namespace news{
    namespace base{


       struct base_operation{
           void get_sign_name(flat_set<account_name> &names) const{};
           void get_sign_owner(flat_set<account_name> &names) const{};
           void validate() {}
           bool is_virtual(){return false;}
       };

        struct create_account_operation : public base_operation{
            account_name    name;
            account_name    creator;
            shared_authority posting;
            shared_authority owner;
            void validate() const;
            void get_sign_name(flat_set<account_name> &names) const { names.insert(creator);}
        };


        struct transfer_operation : public base_operation{ //zhuan zhang 
            account_name    from;
            account_name    to;
            asset           amount;
            std::string     memo;
            void validate() const;
            void get_sign_name(flat_set<account_name> &names) const{ names.insert(from);}
        };


        struct transfers_operation : public base_operation{ //pi liang zhuan zhang
            account_name                    from;
            std::map<account_name, asset>   to_names;
            std::string                     memo;
            void validate() const;
            void get_sign_name(flat_set<account_name> &names) const{ names.insert(from);}
        };


		struct comment_operation : public base_operation {
			account_name   author;
			std::string    title; // 128
			std::string    body; //  1024
			std::string    permlink; //128
			account_name   parent_author;
			std::string    parent_permlink;
			std::string    metajson; 
			void validate() const;
			void get_sign_name(flat_set<account_name> &names) const { names.insert(author); }
		};



		struct comment_vote_operation : public base_operation {
			account_name   voter; 
			account_name    author;
			std::string    permlink; //128	 
			 int 	   ticks ; 
			 std::string       memo;
			void validate() const;
			void get_sign_name(flat_set<account_name> &names) const { names.insert(voter); }
		};
			 
        /********************************************************************
         *                      virtual operation
         ********************************************************************
         * */

        typedef fc::static_variant<create_account_operation,
                transfer_operation,
                transfers_operation,
				comment_operation,
				comment_vote_operation>
                operation;


    }//news::base
}//news


FC_REFLECT_TYPENAME(news::base::operation)

FC_REFLECT(news::base::create_account_operation, (name)(creator)(posting)(owner))
FC_REFLECT(news::base::transfer_operation, (from)(to)(amount)(memo))
FC_REFLECT(news::base::transfers_operation, (from)(to_names)(memo)) 
FC_REFLECT(news::base::comment_operation, (author)(title)(body)(permlink)(parent_author)(parent_permlink)(metajson))
FC_REFLECT(news::base::comment_vote_operation, (voter)(author)(permlink)(ticks)(memo))

/********************************************************************
   *                      virtual operation
   ********************************************************************
   * */



 


