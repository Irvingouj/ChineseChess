#ifndef NODE_H
#define NODE_H
#include <vector>
#include <math.h>
#include <limits>

#define BLACK false
#define RED true

using namespace std;

template<class T>
class Node{

public:
    T state;
    int numPlayed = 0;
    int numWined = 0;
    Node<T>* parent;
    vector<Node*> children;

    Node<T>() = default;
    Node<T>(T s,Node<T>* p):state(s),parent(p){}
    ~Node(){
        for(auto node:children){
            delete node;
        }
        children.clear();
    }

    Node<T>* select(){
        if(this->isLeaf()){
            return this;
        }

        auto nextSelection = this->selectPolicy();
        return nextSelection->select();
    }

    Node<T>* expand(){
        /* should not generate all children, but instead,
         * should keep a list of states that has not been touched
         */
        this->generateChildren();
        // TODO:consider the case where no children can be generated
        return this->expandPolicy();
    }

    bool simulate(){
        return this->state.playoutUntilEnd();
    }

    void backPropagate(bool result){
        if(this->parent == nullptr){
            this->numPlayed += 1;
            return;
        }

        if(state.currentTurn&&result){
            this->numWined +=1;
        }else if(!state.currentTurn && !result){
            this->numWined +=1;
        }
        this->numPlayed += 1;

        parent->backPropagate(result);
    }

    Node<T>* bestChild(){
        Node* res = children[0];
        for (int var = 0; var < children.size(); ++var) {
            if(children[var]->numPlayed > res->numPlayed){
                res = children[var];
            }
        }
        return res;
    }

private:

    Node<T>* selectPolicy(){
        double max = -1;
        Node<T>* res = nullptr;
        for(auto node:this->children){
            auto value =this->UCB(node);
            // if this node hasn't been played, select it;
            if(value == std::numeric_limits<double>::max()){
                return node;
            }
            if(value>max){
                max = value;
                res = node;
            }
        }
        return res;
    }

    double UCB(Node<T>* node){
        if(numPlayed == 0){
            return std::numeric_limits<double>::max();
        }
        return node->numWined/(double)node->numPlayed
                + 1.141 *sqrt(log((double)node->parent->numPlayed)/(double)node->numPlayed);
    }

    void generateChildren(){
        vector<T> states = this->state.getAllPossibleNextState();
        for (auto s:states){
            children.push_back(new Node(s,this));
        }
    }

    Node<T>* expandPolicy(){
        // expand randomly for now, should have some sort of policy to avoid bad node
       for(auto n:this->children){
           if(n->numPlayed == 0){
               return n;
           }
       }
       return this->children[0];
    }

    bool isLeaf(){
        return numPlayed == 0;
    }
};

#endif // NODE_H
