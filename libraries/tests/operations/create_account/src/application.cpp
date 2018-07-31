//
// Created by boy on 18-7-30.
//






#include <test/application.hpp>
#include <iostream>
#include <fc/exception/exception.hpp>
#include <news/chain/transaction.hpp>
#include <boost/lockfree/queue.hpp>
#include <test/factory.hpp>
#include <test/record.hpp>
#include <test/types.hpp>
#include <fc/io/json.hpp>

namespace test{


    using namespace news::chain;


    namespace  detail{

        using namespace news::chain;
        using namespace tools;

//        typedef boost::promise<void>*       promise_ptr;


        struct write_context{
            write_context(){

            }
            ~write_context(){

            }
            std::string         data;

        };








        class application_impl{
        public:
            application_impl():write_queue(256),get_queue(256){

            }
            ~application_impl(){}


//            bpo::options_description    _op_desc;
            bpo::options_description    _cfg_desc;

            bpo::variables_map          _map_args;

            std::vector<std::shared_ptr<http::client>>   _clients;

            uint16_t                    _producer_threads;
            uint32_t                    _trx_ops;
            uint32_t                    _send_type;
            uint32_t                    _second_send;
            uint32_t                    _log_time;




//            std::shared_ptr<std::thread>        _send_threads;
            boost::lockfree::queue<write_context*>  write_queue;
            boost::lockfree::queue<get_context*>    get_queue;


            test::record                    _record;
            factory::create_factory         _create_factory;


            void start_loop();

        };



        void application_impl::start_loop() {

            //发送数据
            for(auto &cc : _clients){
                std::thread([&](){
                    write_context *cxt = nullptr;
                    bool running = true;
                    fc::time_point start = fc::time_point::now();
                    uint32_t    count = _second_send;
                    while(running){
                        try {
                            if(write_queue.pop(cxt)){
                                if(cxt->data.length() > 0
//                                   && cxt->data.length() < 5000
                                   && cxt != nullptr){

                                    get_context *get_cxt = new get_context();
                                    auto send = fc::json::from_string(cxt->data).as<tools::send_body>();
                                    if(send.id == 0){
                                        elog("send id  == 0 .");
                                    }
                                    get_cxt->id = send.id;
                                    get_cxt->send = send;
                                    get_cxt->send_time = fc::time_point::now();
                                    cc->send_message(cxt->data);
                                    get_queue.push(get_cxt);
                                    count--;
                                }

                                if(cxt != nullptr){
                                    delete cxt;
                                    cxt = nullptr;
                                }
                            }

                            if(count <= 0 ){
                                if( (fc::time_point::now() - start).count() < fc::seconds(1).count() ){
                                    while( !((fc::time_point::now() - start).count() > fc::seconds(1).count()) ){
//                                    std::this_thread::sleep_for(2s);
                                    }
//                                    ilog("sleep for. ${t}", ("t", fc::time_point::now() - start));
                                }
                                count = _second_send;
                                start = fc::time_point::now();
//                                ilog("update time ${t}", ("t", start));
                            }

                        }catch (const std::exception &e){
                            wlog("std::exception ${e}", ("e", e.what()));
                        }catch (const fc::exception &e){
                            wlog("${e}", ("e", e.to_detail_string()));
                        }


                    }
                }).detach();
            }

            //handle record message
            //处理接受数据
            std::thread([&](){
                bool running = true;
                get_context *cxt = nullptr;
                fc::time_point start = fc::time_point::now();
                while(running){

                    while(get_queue.pop(cxt)){
                        if(cxt != nullptr){
                            if(cxt->get_time != fc::time_point()){              //更新接受数据
                                _record.update_data(cxt->id, *cxt);
                            }
                            else if(cxt->send_time != fc::time_point()){            //发送数据
                                _record.add_send_data(cxt->id, *cxt);
                            }
                            else{
                                elog("unhandle message data.");
                            }

                            delete  cxt;
                            cxt = nullptr;

                        }
                    }
                    //log data
                    if ( fc::time_point::now() - start > fc::seconds(_log_time) ){
                        _record.log_data_and_move();
                        start = fc::time_point::now();
                    }

                }
            }).detach();

        }


    }





    application::application():my(new detail::application_impl()) {

    }

    void application::set_program_args(int argc, char **argv) {

        my->_cfg_desc.add_options()
                ("help", "help message.")
                ("type", bpo::value<uint32_t>()->default_value(1), "1 create_account, 2 transfer, 3 transfers")
                ("producer-threads", bpo::value<uint16_t>()->default_value(1), "create json threads.")
//                ("threads", bpo::value<uint16_t >()->default_value(1), "")
                ("websocket,w", bpo::value<std::vector<std::string>>(), "webserver address.")
                ("trx-ops", bpo::value<uint32_t>()->default_value(1), "how many operations in transactions")
                ("second-send", bpo::value<uint32_t>()->default_value(500), "every second send trx")
                ("log-time", bpo::value<uint32_t>()->default_value(10), "how long log data.(second)");



        bpo::store(bpo::parse_command_line(argc, argv, my->_cfg_desc), my->_map_args);
//        bpo::store(bpo::parse_command_line(argc, argv, _op_desc), _map_args);



    }

    void application::start() {
        try {
            if( my->_map_args.count("help")){
                std::cout << my->_cfg_desc << std::endl;
                return;
            }
            if( my->_map_args.count("websocket")){
                std::vector<std::string> address =  my->_map_args["websocket"].as<std::vector<std::string>>();
                FC_ASSERT(address.size() > 0, "must once server address.");

                for(const auto &ad : address){
                    auto client = std::make_shared<http::client>(ad);
                    client->init();
                    client->set_handle_message([&](const std::string &str){
                        handle_message(str);
                    });
                    my->_clients.emplace_back(std::move(client));

                }
            }else{
                std::cout << "must init server address" << std::endl;
                return;
            }

            my->_producer_threads = my->_map_args["producer-threads"].as<uint16_t>();
            my->_trx_ops = my->_map_args["trx-ops"].as<uint32_t>();
            my->_send_type = my->_map_args["type"].as<uint32_t>();
            my->_second_send = my->_map_args["second-send"].as<uint32_t>();
            my->_log_time = my->_map_args["log-time"].as<uint32_t>();



            factory::producer_type type;
            switch (my->_send_type){
                case 1:
                    type = factory::producer_type::create_accounts;
                    break;
                case 2:
                    type = factory::producer_type::create_transfer;
                    break;
                case 3:
                    type = factory::producer_type::create_transfers;
                    break;

                default:
                    elog("unkown type ${e}", ("e", my->_send_type));
                    assert(false);
            }


            my->_create_factory.update_param(type, 1, my->_trx_ops, my->_second_send + 10);



            my->_create_factory.set_producer_data_call([&](std::vector<signed_transaction> trx){
                auto ret = my->_record.add_record_data(trx);
                for(auto &r : ret){
                    auto cxt = new detail::write_context();
                    cxt->data = r;
                    my->write_queue.push(cxt);
                }
            });

            my->start_loop();


            std::thread([&](){
                my->_create_factory.start();
            }).join();


        }catch (const fc::exception &e){
            std::cerr << e.to_detail_string() << std::endl;
        }catch (const std::exception &e){
            std::cerr << e.what() << std::endl;
        }




    }

    void application::handle_message(const std::string &message) {
        try {
            auto get_cxt = new detail::get_context();

            auto ret = fc::json::from_string(message).as<tools::result_body>();
            get_cxt->id = ret.id;

            get_cxt->get_time = fc::time_point::now();
            get_cxt->ret = ret;
            my->get_queue.push(get_cxt);
        }FC_CAPTURE_AND_LOG((message))




    }

    application::~application() {
        my.reset();
    }




}