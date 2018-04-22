#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <stack>

namespace bt
{

  struct bt_node
  {
    bool is_leaf;
    uint32_t cur_size;
    std::vector<std::pair<uint32_t, uint32_t> > values;
    std::vector<bt_node*> ptrs;
  };

  class btree
  {
  
    public:
      btree(uint32_t degree);
      bool search_kv(uint32_t key, uint32_t &value);
      bool insert_kv(uint32_t key, uint32_t value);
      //bool delete_kv(uint32_t key);

      //Utitly functions
      int64_t bsearch_key(bt_node* n, uint32_t key);
      bt_node* init_bt_node(bool type);
      bt_node* get_root();
    
      //Debugging 
      void print_tree(bt_node* n);
      void print_node(bt_node* n);
    
    private:
      uint32_t degree;
      bt_node *root;
  };

} //namespace bt
