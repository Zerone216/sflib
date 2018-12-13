
/**
* @version 1.0.0
* @author skyfire
* @mail skyfireitdiy@hotmail.com
* @see http://github.com/skyfireitdiy/sflib
* @file sf_tcp_nat_traversal_utils.h

* sflib第一版本发布
* 版本号1.0.0
* 发布日期：2018-10-22
*/

#pragma once

#include "sf_stdc++.h"
#include "sf_tcp_client.hpp"
#include "sf_tcp_server.hpp"
#include "sf_type.hpp"
#include "sf_serialize_binary.hpp"
#include "sf_net_utils.hpp"
#include "sf_object.hpp"
#include "sf_nocopy.h"
#include "sf_logger.hpp"

namespace skyfire {

    /**
     * @brief nat穿透连接上下文
     */
    struct sf_tcp_nat_traversal_context_t__ {
        int connect_id;
        unsigned long long src_id;
        sf_addr_info_t src_addr;
        unsigned long long dest_id;
        sf_addr_info_t dest_addr;
        int error_code;
        int step;
        bool raw;
    };

    SF_MAKE_SERIALIZABLE_BINARY(sf_tcp_nat_traversal_context_t__, connect_id, src_id, src_addr, dest_id, dest_addr, error_code,
                         step, raw)

    /**
     *  @brief nat穿透连接类型
     */
    enum class sf_tcp_nat_traversal_connection_type {
        type_server_valid,
        type_client_valid
    };


    class sf_tcp_nat_traversal_client;

    /**
     *  @brief nat连接对象
     */
    class sf_tcp_nat_traversal_connection : public sf_nocopy<sf_object> {

        /**
         * 数据到来信号
         */
        SF_REG_SIGNAL(data_coming, const sf_pkg_header_t &, const byte_array &)
        /**
         * 原始数据到来信号
         */
        SF_REG_SIGNAL(raw_data_coming, const byte_array &)
        /**
         * 关闭信号
         */
        SF_REG_SIGNAL(closed)

    private:

        std::shared_ptr<sf_tcp_client> client__;
        std::shared_ptr<sf_tcp_server> server__;
        SOCKET sock__;
        int connect_id__;
        sf_tcp_nat_traversal_connection_type type__;


        sf_tcp_nat_traversal_connection(std::shared_ptr<sf_tcp_client> client,
                                        std::shared_ptr<sf_tcp_server> server,
                                        int sock,
                                        int connect_id,
                                        sf_tcp_nat_traversal_connection_type type);

    public:
        /**
         * 发送数据包
         * @param type 类型
         * @param data 数据
         * @return 是否成功
         */
        bool send(int type, const byte_array &data);

        /**
         * 发送数据
         * @param data 数据
         * @return 是否成功
         */
        bool send(const byte_array &data);

        friend sf_tcp_nat_traversal_client;
    };
}

