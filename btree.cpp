#include "btree.h"

//Represents number values. Implies there are DEG+1 pointers
#define DEG 2 
#define LEAF true
#define INNER false
#define STEAL 0
#define MERGE 1

namespace bt 
{

  //Constructor 
  btree::btree(uint32_t degree) 
  {
    this->degree = degree;
    this->root = NULL;
  }


  bt_node* btree::get_root() 
  {
    return root;
  }


  //Binary search for key in a given node
  //Returns the index of the either the  /
  //exact match index or a index of a key/
  //whose value is next least value for  /
  //the given key.
  int64_t btree::bsearch_key(bt_node *n, uint32_t key) 
  {
    
    int32_t start_idx = 0, end_idx = n->values.size()-1;

    while(end_idx >= start_idx)
    {
      uint32_t mid_idx = (start_idx + end_idx)/2;

      if(n->values[mid_idx].first == key) 
      {
        if(n->is_leaf) return mid_idx;
        else return mid_idx+1;
      }
      else if(n->values[mid_idx].first < key)
      {
        start_idx = mid_idx+1;
      }
      else
      {
        end_idx = mid_idx-1;
      }
    }
 
    return start_idx;
  }

  //Search the corresponding value for a given key
  //True: Found and value contains the find
  //False: Not found
  bool btree::search_kv(uint32_t key, uint32_t &value)
  {

    bt_node *temp = this->root;

    while(temp) 
    { 
      int64_t idx = bsearch_key(temp, key);
      if(temp->is_leaf) 
      {
        if(temp->values[idx].first == key) 
        {
          value = temp->values[idx].second;
          return true;
        }
        return false;
      }
      else 
      {
        temp = temp->ptrs[idx];
      }
    }

    return false;
  }

  //Creates bt_node and does basic init 
  bt_node* btree::init_bt_node(bool type)
  {
    bt_node *n = new bt_node;
    n->is_leaf = type;
    //n->values.size() = 0;
    return n;
  }

  //Inserts the give key-value pair to the btree
  bool btree::insert_kv(uint32_t key, uint32_t value) 
  {
    bt_node *temp_node = NULL;
    int64_t idx = 0;
    std::stack<bt_node*> stk;

    //Creating root node, if not alreay available
    if(!root) 
    {
      root = init_bt_node(LEAF);
      root->values.push_back(std::make_pair(key, value));
      root->ptrs.insert(root->ptrs.begin(), {NULL, NULL});
      //root->values.size()++;
      return true;
    }
   
    //Finding the leaf node for initial insert /
    //prior to spliting, if needed
    temp_node = root;
    stk.push(temp_node);
    while(temp_node) 
    {
      idx = bsearch_key(temp_node, key);
      if(temp_node->is_leaf) 
        break;
      temp_node = temp_node->ptrs[idx];
      stk.push(temp_node);
    }
    
    if(!temp_node) 
      return false;


    //Inserting to the leaf node
    temp_node->values.insert(temp_node->values.begin()+idx,
                            std::make_pair(key, value));
    temp_node->ptrs.push_back(NULL);
    //temp_node->values.size()++;
    stk.pop();


    //Keep spliting till the tree is normalized
    //while(temp_node->values.size() >= degree)
    while(temp_node->values.size() >= degree)
    {
      bt_node *parent = NULL;
      bt_node *sib_node = init_bt_node(temp_node->is_leaf);
      //uint32_t sidx = temp_node->values.size()/2;
      uint32_t sidx = temp_node->values.size()/2;
      uint32_t temp_key = temp_node->values[sidx].first;
      uint32_t temp_value = temp_node->values[sidx].second;
      uint32_t it;

      //Moving the half the key-value pairs to new node
      if(sib_node->is_leaf) 
      { 
        //LEAF node
        sib_node->ptrs.push_back(NULL);
        for(it = sidx; it < temp_node->values.size(); ++it) 
        {
          sib_node->values.push_back(std::move(temp_node->values[it]));
          sib_node->ptrs.push_back(NULL); //No pointers to move. 
          //sib_node->values.size()++;
        }
      } 
      else 
      { 
        //INNER node
        for(it = sidx+1; it < temp_node->values.size(); ++it)
        {
          sib_node->values.push_back(std::move(temp_node->values[it]));
          sib_node->ptrs.push_back(std::move(temp_node->ptrs[it]));
          //sib_node->values.size()++;
        }
        sib_node->ptrs.push_back(std::move(temp_node->ptrs[it]));
      }
      //temp_node->values.size() -= (sidx+1);
      temp_node->values.erase(temp_node->values.begin()+sidx,
                                            temp_node->values.end());

      //Inserting the new sibling to the parent node
      
      //Root node requires split
      if(stk.empty()) 
      {
        bt_node *new_root = init_bt_node(INNER);
        new_root->values.push_back(std::make_pair(temp_key, temp_value));
        new_root->ptrs.insert(new_root->ptrs.begin(), {temp_node, sib_node});
        //new_root->values.size()++;
        root = new_root; //New root node
        return true;
      } 

      //Inner node requires split
      parent = stk.top();
      stk.pop();
      
      idx = bsearch_key(parent, temp_key);
      parent->values.insert(parent->values.begin()+idx, 
                          std::make_pair(temp_key, temp_value));
      parent->ptrs.insert(parent->ptrs.begin()+idx+1,
                          sib_node);
      //parent->values.size()++;
      temp_node = parent;
    }
    return true;
  }

 
  bool btree::delete_kv(uint32_t key) {
    bt_node *temp_node = NULL;
    std::stack<std::pair<bt_node*, uint32_t> > stk; 
    
    if(!root) 
      return false;

    //Search for the leaf node
    temp_node = root;
    while(temp_node) 
    {
      idx = bsearch_key(temp_node, key);
      stk.push(std::make_pair(temp_node,idx));
      
      if(temp_node->is_leaf) 
      {
        stk.pop();
        break;
      }

      temp_node = temp_node->ptrs[idx];
    }

    //Key not found in leaf node
    if(temp->values[idx].first != key) 
      return false;

    //Deleted the key value pair from the leaf node
    temp->values.erase(temp->values.begin()+idx);
    temp->ptrs.erase(temp->ptrs.begin()+idx+1);
    //temp->values.size()--;

    //Root is leaf node or the selected leaf node 
    //still satisfies btree requirements even after 
    //deletion
    //if(temp == root || temp->values.size() >= (DEG-1)/2) 
    //  return true;

    //while(temp != root || temp->values.size() < (DEG-1)/2) 
    while(temp != root || temp->values.size() < DEG/2) 
    {

      bt_node *parent = NULL, *sibbling = NULL;

      //Adjustment is required after deletion
      //1. Steal
      //2. Merge
      parent = stk.top().first;
      parent_idx  = stk.top().second;
      stk.pop();

      int64_t left_sib = -1, right_sib = -1;
      int64_t sib_idx = -1;

      //Number of elements required by the target node to be half full
      req_size = (DEG/2) - temp->values.size();

      //Pick a sibbling to try stealing from
      if(parent_idx-1 >= 0) 
        left_sib  = parent_idx-1;
      else if(parent_idx+1 <= parent->values.size()) 
        right_sib = parent_idx+1;
      
      if(left_sib == -1) sib_idx = right_sib;
      else if(right_sib == -1) sib_idx = left_sib;
      else
      {
        sib_idx = (parent->ptrs[left_sib]->values.size() 
                >= parent->ptrs[right_sib]->values.size())?left_sib:right_sib;
      }

      //Decide to steal or merge
      sibbling = parent->ptrs[sib_idx];
      //Computes the size of the sibbling after stealing 
      //Decides to steal or merge
      mode = ((sibbling->values.size() - req_size) >= (DEG/2))?STEAL:MERGE;
      
      switch(mode)
      {
        case STEAL:
          //Sibbling is left node
          if(sib_idx < parent_idx)
          {
            uint32_t iter;
            for(iter=sibbling->values.size()-1; 
                iter>sibbling->values.size()-1-req_size; iter--) 
            {
              if(temp->is_leaf) 
              {
                temp->values.insert(temp->values.begin(),
                          std::move(sibbling->values[iter]));
                temp->ptrs.insert(temp->ptrs.begin(), NULL);             
                parent->values[parent_idx-1] = temp->values[0];
              }
              else
              {
                temp->values.insert(temp->values.begin(),
                          std::move(parent->values[parent_idx-1]));
                temp->ptrs.insert(temp->ptrs.begin(),
                          std::move(sibbling->ptrs[iter+1]));
                parent->values.insert(parent->values.begin()+parent_idx-1,
                          std::move(sibbling->values[iter]));
              }
            }
            sibbling->values.erase(sibbling->values.begin()+(sibbling->values.size()-req_size-1),
                                   sibbling->values.end());
            sibbling->ptrs.erase(sibbling->ptrs.begin()+(sibbling->ptrs.size()-req_size),
                                 sibbling->ptrs.end());

          }
          else //Sibbling is right node
          {
            uint32_t iter;
            for(iter=0; iter < req_size; iter++)
            {
              if(temp->is_leaf)
              { 
                temp->values.insert(temp->values.end(), 
                                    std::move(sibbling->values[iter]));
                temp->ptrs.insert(temp->ptrs.end(), NULL);
                parent->values[parent_idx] = sibbling->values[0];
              }
              else
              {
                 temp->values.insert(temp->values.end(),
                                     std::move(parent->values[parent_idx]));
                 temp->ptrs.insert(temp->ptrs.end(),
                                   std::move(sibbling->ptrs[iter]));
                 parent->valuses.insert(parent->values.begin()+parent_idx,
                                        std::move(sibbling->values[iter]));
              }
            }
            sibbling->values.erase(sibbling->values.begin(),
                                   sibbliig->values.begin()+req_size);
            sibbling->ptrs.erase(sibbling->ptrs.begin(),
                                 sibbling->ptrs.begin()+req_size);
          }
          break;

        case MERGE:
          bt_node *to, *from;
          uint32_t tmp_pidx = 0;

          //Recompute sib_idx
          if(left_sib == -1) sib_idx = right_sib;
          else if(right_sib == -1) sib_idx = left_sib;
          else
          {
            sib_idx = (parent->ptrs[left_sib]->values.size() 
                       <= parent->ptrs[right_sib]->values.size())?left_sib:right_sib;
          }
          
          if(sib_idx < parent_idx) 
          {
            to = parent->ptrs[sib_idx];
            from = temp;
            tmp_pidx = parent_idx-1;
          }
          else 
          {
            to = temp;
            from = parent->ptrs[sib_idx];
            tmp_pidx = parent_idx;
          }

          if(to->is_leaf) 
          {
            for(auto val : from->values)
            {
              to->values.insert(to->values.end(), std::move(val));
              to->ptrs.insert(to->ptrs.end(), NULL);
            }
          }
          else
          {
            uint32_t iter = 0;
            to->values.insert(to->values.end(), 
                              std::move(parent->values[tmp_pidx]));
            for(auto val : from->values)
            {
              to->values.insert(to->values.end(), std::move(val));
              to->ptrs.insert(to->ptrs.end(), std::move(from->ptrs[iter]);
              iter++;
            }
            to->ptrs.insert(to->ptrs.end(), std::move(from->ptrs[iter]);
          }
            
          parent.erase(parent->values.begin()+tmp_pidx);
          parent.erase(parent->ptrs.begin()+parent_idx);
          delete from;
           
          break;
      }

      temp = parent;
    }

    return true;
  
  }


  void btree::print_node(bt_node *n){
    uint32_t it;
    
    for(it=0;it<n->values.size();it++)
    {
      std::cout<<n->values[it].first<<"\t";
    }
    std::cout<<std::endl;
  }

  void btree::print_tree(bt_node *n)
  {
    uint32_t i = 0;
    if(!n) 
      return;
   
    for(i=0;i<n->values.size();i++)
    { 
        print_tree(n->ptrs[i]);
        std::cout<<n->values[i].first<<std::endl;
    }
    print_tree(n->ptrs[i]);
  }

};

using namespace bt;

int main()
{

    if(DEG < 2) return -1;
    btree *bt = new btree(DEG);
    uint32_t val = 0;
    bt->insert_kv(95,1);
    bt->insert_kv(21,2);
    bt->insert_kv(7,3);
    bt->insert_kv(17,4);
    bt->insert_kv(44,5);
    bt->insert_kv(92,6);
    bt->print_tree(bt->get_root());

    if(bt->search_kv(44, val))
      std::cout<<"Search Key = 44, Value = "<<val<<std::endl;

    return 0;
}
