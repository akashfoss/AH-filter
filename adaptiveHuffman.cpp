#include <iostream>
#include <bitset>
#include "adaptiveHuffman.h"

AdaptiveHuffmanModel::AdaptiveHuffmanModel() {
    Construct(false);
}

AdaptiveHuffmanModel::AdaptiveHuffmanModel(bool s) {
    Construct(s);
}

void AdaptiveHuffmanModel::Construct(bool s) {
    split = s;
    root = new Node();
    nyt = root;
    root->number = 255;
    startBlock = new Block(false, 0);
    startBlock->leader = root;
    startBlock->tail = root;
    root->block = startBlock;

    Block* zeroInternalBlock = new Block(true, 0);
    startBlock->next = zeroInternalBlock;
    zeroInternalBlock->prev = startBlock;
}

AdaptiveHuffmanModel::~AdaptiveHuffmanModel() {
    delete root;
    delete startBlock;
}

AdaptiveHuffmanModel::Node::Node() {
    Construct(0);
}

AdaptiveHuffmanModel::Node::Node(unsigned int w) {
    Construct(w);
}

void AdaptiveHuffmanModel::Node::Construct(unsigned int w) {
    weight = w;
    symbol = 0;
    lchild = NULL;
    rchild = NULL;
    parent = NULL;
    next = NULL;
    prev = NULL;
    block = NULL;
}

AdaptiveHuffmanModel::Node::~Node(){
    if (lchild) {
        delete lchild;
    }
    if (rchild) {
        delete rchild;
    }
}

AdaptiveHuffmanModel::Block::Block(){
    Construct(true, 0);
}

AdaptiveHuffmanModel::Block::Block(bool intern){
    Construct(intern, 0);
}

AdaptiveHuffmanModel::Block::Block(bool intern, unsigned int w){
    Construct(intern, w);
}

void AdaptiveHuffmanModel::Block::Construct(bool intern, unsigned int w){
    internal = intern;
    weight = w;
    next = NULL;
    prev = NULL;
    leader = NULL;
    tail = NULL;
}

void AdaptiveHuffmanModel::Block::insert(Node* node){
    if (tail){
        tail->prev = node;
        node->next = tail;
    }
    if (!leader){
        leader = node;
    }

    tail = node;
    node->block = this;
}

void AdaptiveHuffmanModel::Block::remove(Node* node){
    if (node->prev){
        node->prev->next = node->next;
    }
    if (node->next){
        node->next->prev = node->prev;
    }
    if (leader == node){
        leader = node->prev;
    }
    if (tail == node){
        tail = node->next;
    }
    node->prev = NULL;
    node->next = NULL;
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::splitNYT() {
    Node* newLeaf = new Node();
    nyt->rchild = newLeaf;
    newLeaf->parent = nyt;
    newLeaf->number = nyt->number-1;

    nyt->lchild = new Node();
    nyt->lchild->number = nyt->number-2;
    nyt->lchild->parent = nyt;

    // Move the old NYT node from external 0-weight block to internal 0-weight
    // block
    nyt->block->remove(nyt);
    startBlock->next->insert(nyt);

    nyt = nyt->lchild;

    startBlock->insert(newLeaf);
    startBlock->insert(nyt);

    return newLeaf;
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::findNode(unsigned char c){
    return findNodeRecursive(c, root);
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::findNodeRecursive(unsigned char c, AdaptiveHuffmanModel::Node* node){
    if (!node){
        return NULL;
    }

    if (node->symbol == c){
        return node;
    }
    
    Node* returnNode = findNodeRecursive(c, node->rchild);
    if (returnNode){
        return returnNode;
    }
    return findNodeRecursive(c, node->lchild);
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::findMaxInBlock(unsigned int weight){
    return findMaxInBlockRecursive(weight, root, 0);
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::findMaxInBlockRecursive(unsigned int weight, AdaptiveHuffmanModel::Node* currNode, unsigned char bestNum){
    if (!currNode){
        return NULL;
    }

    if (currNode->weight == weight){
        return currNode;
    } else if (currNode->weight < weight || currNode->number < bestNum){
        return NULL;
    }

    Node* returnNode = NULL;
    Node* rightNode = findMaxInBlockRecursive(weight, currNode->rchild, bestNum);
    if (rightNode){
        if (rightNode->number > bestNum){
            returnNode = rightNode;
            bestNum = rightNode->number;
        }
    }

    Node* leftNode = findMaxInBlockRecursive(weight, currNode->lchild, bestNum);
    if (leftNode){
        if (leftNode->number > bestNum){
            return leftNode;
        }
    }

    return returnNode;
}

AdaptiveHuffmanModel::Block* AdaptiveHuffmanModel::insertNodeIntoBlock(Node* node, bool internal){
    Block* currBlock = startBlock;
    while (currBlock->weight != node->weight){
        if (!currBlock->next){
            Block* temp = new Block(internal, node->weight);
            temp->prev = currBlock;
            currBlock->next = temp;

            currBlock = temp;
            break;
        } else {
            currBlock = currBlock->next;
            if (currBlock->weight > node->weight){
                Block* temp = new Block(internal, node->weight);
                temp->next = currBlock;
                temp->prev = currBlock->prev;
                currBlock->prev->next = temp;
                curr->prev = temp;

                currBlock = temp;
                break;
            }
        }
    }

    if (node->block){
        node->block->remove(node);
    }

    currBlock->insert(node);
    return currBlock;
}

AdaptiveHuffmanModel::Node* AdaptiveHuffmanModel::addSymbol(unsigned char c){
    Node* newLeaf = splitNYT();
    newLeaf->weight = 1;
    newLeaf->symbol = c;

    Block* newLeafBlock = newLeaf->block->next;
    newLeaf->block->remove(newLeaf);

    if (newLeafBlock){
        if (startBlock->weight != 1 || startBlock->internal){
            Block* temp = new Block(false, 1);
            temp->next = newLeafBlock;
            newLeafBlock->prev->next = temp;
        } else {
            newLeafBlock->insert(newLeaf);
        }
    } else {
        newLeafBlock = new Block(false, 1);
        newLeafBlock = newLeaf->block;


    newLeaf->parent->weight = 1;

    return newLeaf->parent;
}

// Switches the position of two nodes, while leaving the numbering intact
void AdaptiveHuffmanModel::switchNodes(Node* node1, Node* node2){
    unsigned char tempNum = node1->number;
    Node* tempParent = node1->parent;

    node1->number = node2->number;
    node1->parent = node2->parent;
    if (node1->parent->lchild == node2){
        node1->parent->lchild = node1;
    } else {
        node1->parent->rchild = node1;
    }

    node2->number = tempNum;
    node2->parent = tempParent;
    if (tempParent->lchild == node1){
        tempParent->lchild = node2;
    } else {
        tempParent->rchild = node2;
    }
}

void AdaptiveHuffmanModel::blockSwitch(Node* node){
    Node* maxBlockNode = findMaxInBlock(node->weight);
    if (node != maxBlockNode && maxBlockNode != node->parent){
        switchNodes(node, maxBlockNode);
    }
    node->weight++;
}

std::string AdaptiveHuffmanModel::encode(unsigned char c){
    std::string output = "";
    Node* node = findNode(c);
    if (!node){
        output = nodeToString(nyt);
        std::bitset<8> bs(c);
        output += bs.to_string();
    } else {
        output = nodeToString(node);
    }

    return output;
}

std::string AdaptiveHuffmanModel::nodeToString(AdaptiveHuffmanModel::Node* node){
    std::string output = "";

    while (node != root){
        output = (node == node->parent->rchild) ? "1" + output : "0" + output;
        node = node->parent;
    }

    return output;
}

std::string AdaptiveHuffmanModel::decode(std::string message) {
    Node* currNode = root;
    std::string::iterator it = message.begin();
    std::string decodedMessage = "";
    unsigned char bitChar;
    unsigned char decodedChar;

    while (it != message.end()){
        if (currNode == nyt){
            decodedChar = 0;
            for (unsigned char i = 0; i < 8; i++){
                bitChar = *it++;
                decodedChar <<= 1;
                if (bitChar == '1'){
                    decodedChar += 1;
                }
            }
            decodedMessage += decodedChar;
            updateModel(decodedChar);
            currNode = root;
        } else {
            bitChar = *it++;
            if (bitChar == '1'){
                currNode = currNode->rchild;
            } else if (bitChar == '0') {
                currNode = currNode->lchild;
            }
            if (currNode->symbol){
                decodedChar = currNode->symbol;
                decodedMessage += decodedChar;
                updateModel(decodedChar);
                currNode = root;
            }
        }
    }

    return decodedMessage;
}

// TODO Make invariant
void AdaptiveHuffmanModel::updateModel(unsigned char c) {
    Node* currNode = findNode(c);
    if (!currNode){
        currNode = addSymbol(c);
    } else {
        blockSwitch(currNode);
    }

    while (currNode != root){
        currNode = currNode->parent;
        blockSwitch(currNode);
    }
    printTree(root, 0);
}

void AdaptiveHuffmanModel::printTree(AdaptiveHuffmanModel::Node* node, int indent) {
    if (!indent){
        std::cout << std::endl;
    }
    for (int i = 0; i < indent; i++){
        std::cout << "  ";
    }
    std::cout << "Number: " << (int) node->number << "  " << "Weight: " << node->weight << "  Symbol: ";
    if (node->symbol){
        std::cout << node->symbol << std::endl;
    } else {
        std::cout << "null" << std::endl;
    }
    if (node->rchild) {
        printTree(node->rchild, indent+1);
    }
    if (node->lchild) {
        printTree(node->lchild, indent+1);
    }
}
