#ifndef STORAGE_CONFIG_MESSAGE_TYPE_H_
#define STORAGE_CONFIG_MESSAGE_TYPE_H_

// Message types for network communication
// Messages used for request are represented in lowercase letters.
// And messages used for reply are represented in uppercase letters

#include <memory>
#include <string>

typedef char msg_t;

// e.g. msg: "PUT_DATA\tkey\tvalue\n"
// Client
#define GET_HASH 'z'  // Request hash distribution from Master.
#define GET_DATA 'y'  // Request to read data from CacheServer.
#define PUT_DATA 'x'  // Request to store data to CacheServer.
#define REC_HASH 'w'  //
#define IAM_CLNT 'v'  // Notify server this is a client.
#define WRI_FINI 'u'  // Notify the read client that write is finish
#define SET_BUSY 't'
// CacheServer
#define RET_ADDR 'A'  // Return an address of a specific CacheServer to Client.
#define NEW_HASH 'B'  // Notify CacheServer of new hash slot.
#define UPG_PMRY 'C'  // Notify CacheServer to upgrade to primary node.
#define MGR_STAR 'D'  // Notify CacheServer to migrate its data.
#define URE_PRIM 'E'  // Tell CacheServer you are a primary node.
#define URE_STDB 'F'  // Tell CacheServer you are a standby node.
#define CON_STDB 'G'  // Notify primary cache to connect to its standby node.
#define EXP_LSTN 'H'  // notify the new cache to set listen for incoming data from old cache server
#define NEW_CACH 'I'  // Tell client new cache
#define DEL_CACH 'J'
#define CON_LSTN 'K'  // notify the old cache to set listen for incoming data from dying cache server
#define EXP_LTSN 'L'
#define RET_ADRS 'M'  // Tell dying CacheServer the addresses of alive CacheServers.
#define MGR_SUCC 'N'
#define CON_SHUT 'O'
#define LRU_BUSY 'P' // Tell the client LRU is writing
// Master
#define RET_VALE 'a'  // Return the value of a specific key to Client.
#define PUT_BACD 'b'  // Send data to standy node for backup.
#define REP_HTBT 'c'  // Report heartbeat to Master.
#define REC_DATA 'd'  // Request to recover data from primary CacheServer.
#define REC_COMP 'e'  // Notify the primary node that data recovery has completed.
#define MGR_COMP 'f'  // Notify master that data migration has complete.
#define MGR_FAIL 'g'  // Notify master that data migration has failed.
#define STO_SUCC 'h'  // Notify client data store success.
#define IAM_CACH 'i'  // Notify master that this is a cache but not client.
#define BAC_SUCC 'j'  // Notify the primary node that the data has been backuped.
#define REC_NECA 'k'
#define REC_OTCA 'l'


// Functions for analyzing message.
namespace mt {
msg_t GetMsgType(const std::string& message);
std::string Get1stStr(const std::string& message);
std::string Get2ndStr(const std::string& message);
}  // namespace mt

#endif  // STORAGE_CONFIG_MESSAGE_TYPE_H_