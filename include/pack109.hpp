#include <vector>
#include <string>

#ifndef PACK109_HPP
#define PACK109_HPP

typedef unsigned char u8;
typedef std::vector<u8> vec;
typedef std::string string;

#define PACK109_U8    0xa2
#define PACK109_S8    0xaa
#define PACK109_A8    0xac
#define PACK109_M8    0xae

struct File {
  string name;
  vec bytes;
};

struct Request {
  string name;
};

namespace pack109 {
  vec serialize(u8 item);
  u8 deserialize_u8(vec bytes);

  vec serialize(string item);
  string deserialize_string(vec bytes);
  
  vec serialize(std::vector<u8> item);
  std::vector<u8> deserialize_vec_u8(vec bytes);

  vec serialize(struct File item);
  struct File deserialize_file(vec bytes);

  vec serialize(struct Request item);
  struct Request deserialize_request(vec bytes);

  void printVec(vec &bytes);
}

#endif
