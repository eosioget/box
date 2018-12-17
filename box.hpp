#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <map>
#include <set>
#include <cstdlib>

using eosio::key256;
using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;

#define DEBUG

class box : public eosio::contract {
public:
    box(account_name self) : eosio::contract(self),
        pool(_self, _self),
        games(_self, _self),
        offers(_self, _self)
    { }

    //@abi action
    void creategame(const uint32_t roomid, const uint64_t gameid, account_name creator, const asset& amount);
    //@abi action
    void revealcreate(const uint64_t gameid, const std::string& result, const checksum256& block_id, const uint64_t new_gameid, account_name new_creator);
    //@abi action
    void cleargame(uint64_t update_time_int);
    //@abi action
    void clearoffer(uint64_t update_time_int);
    //@abi action
    void closecreate(const uint64_t gameid, const uint64_t new_gameid);

    void transfer(account_name from, account_name to, const asset& quantity, const std::string& memo);//注意没有action

private:
    //@abi table lucky i64 幸运池
    struct lucky {
        uint64_t id;  // 赌局id，如:20180913 001 00035，中间一段是房号
        asset lucky_total; // 累计金额
        asset lucky_pay; // 累计支出
        asset lucky_pool; // 累计当前金额
        uint64_t update_time_int = current_time(); // 更新时间(int)
        uint64_t backup1 = 0; // 备用字段
        
        uint64_t primary_key() const { return id; }
        uint64_t by_update_time_int() const { return update_time_int; } 
        
        EOSLIB_SERIALIZE(lucky, (id)(lucky_total)(lucky_pay)(lucky_pool)(update_time_int)(backup1) )
    };

    typedef eosio::multi_index< N(lucky), lucky,
        indexed_by< N(update_time_int), const_mem_fun<lucky, uint64_t, &lucky::by_update_time_int> >
    > lucky_index;

    //@abi table game i64 赌局表
    struct game {
        uint64_t id;  // 赌局id，如:20180913 001 00035，中间一段是房号
        uint64_t room = 1; // 房间号
        uint64_t player_num = 5; // 玩家数
        account_name creator; // 红包的创建人
        asset amount; // 金额
        uint32_t ttl = 600; // 超时时时间，默认600秒
        uint8_t game_state = 0; // 状态：0:进行中,  2:已开奖, 3:已关闭
        uint64_t block_num;     // 区块号
        checksum256 block_id;   // 区块id做为开奖种子
        std::string result; // 权重结果
        eosio::time_point_sec create_time; // 创建时间
        uint64_t update_time_int = current_time(); // 更新时间(int)
        uint64_t backup1 = 0; // 备用字段

        uint64_t primary_key() const { return id; }
        uint64_t by_room() const { return room; } // 可以通过房间查询数据
        uint64_t by_update_time_int() const { return update_time_int; } 

        EOSLIB_SERIALIZE(game, (id)(room)(player_num)(creator)(amount)(ttl)(game_state)(block_num)(block_id)(result)(create_time)(update_time_int)(backup1) )
    };

    typedef eosio::multi_index< N(game), game,
        indexed_by< N(room), const_mem_fun<game, uint64_t, &game::by_room> >,
        indexed_by< N(update_time_int), const_mem_fun<game, uint64_t, &game::by_update_time_int> >
    > game_index;

    //@abi table offer i64 赌注表
    struct offer {
        uint64_t id;  // 
        account_name player; // 开宝箱的玩家
        asset amount; // 抢到的金额
        asset lucky_amount; // 幸运奖的金额
        uint64_t game_id;  // 游戏id
        uint8_t result_weight = 0; // 权重
        eosio::time_point_sec create_time; // 创建时间
        uint64_t update_time_int = current_time(); // 更新时间(int)
        uint64_t backup1 = 0; // 备用字段

        uint64_t primary_key() const { return id; }
        uint64_t by_gameid() const { return game_id; } // 可以通过赌局id查询数据
        account_name by_account_name() const { return player; } // 可以通过用户名查询数据
        uint64_t by_update_time_int() const { return update_time_int; } 

        EOSLIB_SERIALIZE(offer, (id)(player)(amount)(lucky_amount)(game_id)(result_weight)(create_time)(update_time_int)(backup1) )
    };

    typedef eosio::multi_index< N(offer), offer,
        indexed_by< N(gameid), const_mem_fun<offer, uint64_t, &offer::by_gameid > >,
        indexed_by< N(player), const_mem_fun<offer, account_name,  &offer::by_account_name> >,
        indexed_by< N(update_time_int), const_mem_fun<offer, uint64_t, &offer::by_update_time_int> >
    > offer_index;

    struct commit_offer {
        uint64_t id;
        account_name player;
        asset amount;           // 抢到的金额
        asset lucky_amount;     // 幸运奖的金额
        uint8_t result_weight;  // 权重
    };

    game_index games;
    lucky_index pool;
    offer_index offers;

    void docreategame(const uint32_t roomid, const uint64_t gameid, account_name creator, const asset& amount);
    asset str2eos(const std::string& eosnum);

    void deposit(const account_name from, const asset& quantity);
    void withdraw(const account_name to, const asset& quantity);

};

#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
 extern "C" { \
    void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
       if (action == N(onerror)) { \
          eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
       } \
       auto self = receiver; \
       if ((code==N(eosio.token) && action==N(transfer)) || (code==self && (action==N(creategame) || action==N(revealcreate) || action==N(cleargame) || action==N(clearoffer) || action==N(closecreate) )) ) { \
          TYPE thiscontract( self ); \
          switch( action ) { \
             EOSIO_API( TYPE, MEMBERS ) \
          } \
       } \
    } \
 }

EOSIO_ABI_EX(box, (creategame)(revealcreate)(cleargame)(clearoffer)(closecreate)(transfer))
