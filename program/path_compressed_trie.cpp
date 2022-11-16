#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <bitset>
#include <ctype.h>
#include <stack>

using namespace std;

#define ipv4Length 32

int NotFoundCnt = 0;
int max_search_record = 0;
string root_ip = "";

class Node{
    public:
    string data;
    Node* left;
    Node* right;
    Node* parent;
    bool visited;
    char side;
    string segment;

    Node(string val)
    {
        data = val;
        left = NULL;
        right = NULL;
        parent = NULL;
        visited = false;
        side = '\0';
        segment = "";
    }
};

string firstIP_address(string ip, int mask_length)
{
    string first_ip = "";
    for(int i=0; i<mask_length; i++)
    {
        first_ip += ip[i];
    }
    return first_ip;
}

string number2binary(string number_input)
{
    int number = stoi(number_input);
    return bitset<8>(number).to_string();
}

void ip2binary(string type, string address, string* ip_result)
{
    bool slash = false;
    int addr_length = address.length();
    string number = "", ip = "";
    for(int i=0; i<addr_length; i++)
    {
        if(address[i] == '/')
        {
            slash = true;
        }
        if(isdigit(address[i]))
        {
            number += address[i];
        }
        else
        {
            ip += number2binary(number);
            number = "";
        }
    }

    if(type == "ip")
    {
        if(slash)
        {
            int mask_length = stoi(number);
            string first_ip = firstIP_address(ip, mask_length);
            
            ip_result[0] = ip;
            ip_result[1] = first_ip;
        }
        else
        {
            ip += number2binary(number);
            number = "";
            ip_result[0] = ip;
            ip_result[1] = "";
        }
    }
    else
    {
        ip_result[0] = ip;
    }
}

void insert(Node* root, int i, int prefix_length, string prefix, string port){
    if(i == prefix_length){
        if(root->data != "*")
            cout << "WARNING: Prefix Used; Overwrite\n";
        root->data = port;
        return;
    }
    if(prefix[i] == '0'){
        if(root->left == NULL){
            Node* node = new Node("*");
            root->left = node;
            node->parent = root;
            node->side = 'L';
            node->segment = root->segment + "0";         
            root = root->left;
            insert(root, i+1, prefix_length, prefix, port);            
        }
        else{
            root = root->left;
            insert(root, i+1, prefix_length, prefix, port);              
        }
    }
    else{
        if(root->right == NULL){
            Node* node = new Node("*");
            root->right = node;
            node->parent = root;
            node->side = 'R';
            node->segment = root->segment + "1";            
            root = root->right;
            insert(root, i+1, prefix_length, prefix, port);            
        }
        else{
            root = root->right;
            insert(root, i+1, prefix_length, prefix, port);              
        }    
    }
}

int size(Node* node)
{
    if (node == NULL)
        return 0;
    else
        return(size(node->left) + 1 + size(node->right));
}

void traverse_trie(Node* node)
{
    if(node == NULL)
        return;
    traverse_trie(node->left);
    
    cout << node->data << "\n";

    traverse_trie(node->right);
}

Node* build(string file_name)
{
    ifstream infile(file_name);
    string line;
    int cnt = 0;
    string ip, port;
    string* ip_result = new string[2];
    string* port_result = new string[1];
    string prefix = "";
    int prefix_length = 0;
    Node* root = new Node("*");
    while(getline(infile, line))
    {
        infile >> ip >> port;
        if(infile.eof())
            break;
        cnt++;

        ip2binary("ip", ip, ip_result);
        if((cnt % 10000) == 0)
        {
            cout << "#Prefixes = " << cnt << "\n";
            //printf("%s,  %s,   %s,  %d\n", 
            //     ip.c_str(), ip_result[0].c_str(), ip_result[1].c_str(), int(ip_result[1].length()));
        }

        prefix = ip_result[1];
        prefix_length = ip_result[1].length();
        if(prefix_length == 0)
        {
            for(int i=0; i<ip.length(); i++)
            {
                if(ip[i] != '/')
                    root_ip += ip[i];
                else
                    break;
            }
        }
        insert(root, 0, prefix_length, prefix, port);
    }
    cout << size(root) << "\n";
    cout << "End of Building\n";
    //cout << root->data << "\n";
    return root;
}

bool search_tree(Node* node, int prefix_length, string prefix, string test_ip, string target_port)
{
    int search_record = 0;
    if(test_ip == root_ip)
        if(node->data == target_port){
            search_record = 1;
            if(max_search_record < search_record)
                max_search_record = search_record;
            return true;
        }
        
    string prev_prefix_pattern = "";
    string prefix_pattern = "";
    int i = 0;
    
    while(i < prefix_length)
    {
        //cout << "bit: " << i << ", " << prefix[i] << "\n";
        search_record++;
        if(prefix[i] == '0')
        {
            node = node->left;
        }
        else
        {
            //cout << "HERE Right\n";
            node = node->right;
        }

        if(node == NULL)
        {
            //cout << "NULL\n";
            return false;
        }
        else
            prefix_pattern = node->segment;
        
        i += prefix_pattern.length() - prev_prefix_pattern.length();
        prev_prefix_pattern = prefix_pattern;
    
        //cout << i+1 << ": " <<  node->data << "\n";
        //cout << "In Search Tree: " << node->segment << ", " << node->segment.length() << ", " << prefix << "\n";
        if(node->data == target_port){
            if(node->segment.length() == ipv4Length)
            {
                if(node->segment == prefix)
                {
                    if(search_record > max_search_record)
                        max_search_record = search_record;
                    return true;
                }
            }
            else
            {
                if(search_record > max_search_record)
                    max_search_record = search_record;
                return true;
            }
        }
    }
    return false;
}

void test_routing(string test_type, Node* root, string test_ip, string* ip_result, string target_port)
{
    ip2binary("ip", test_ip, ip_result);
    //cout << ip_result[0] << "here\n";
    
    string prefix = ip_result[1];
    int prefix_length = ip_result[1].length();
    //cout << prefix << ", " << prefix_length << "\n";
    if(prefix_length == 0)
    {
        prefix = ip_result[0];
        prefix_length = ip_result[0].length();
    }
    //cout << "\n\nIn searching: " << prefix << ", " << prefix_length << "\n";
    
    if(!search_tree(root, prefix_length, prefix, test_ip, target_port))
    {
        NotFoundCnt++;
        cout << "Not Found #" << NotFoundCnt << ": [IP = " << test_ip << ", Hop Port = " << target_port << "]\n";
    }
    else{
        if(test_type == "solo")
            cout << "Found: [IP = " << test_ip << ", Hop Port = " << target_port << "]\n\n";
    }
}

void search_port(Node* node, int prefix_length, string prefix, string test_ip, int bits_number)
{
    int search_record = 0;    
    if(test_ip == root_ip)
    {
        search_record = 1;
        if(max_search_record < search_record)
            max_search_record = search_record;      
        cout << "Matched Port = " << node->data << "\n";
        return;
    }
    search_record = 0;
    string prev_prefix_pattern = "";
    string prefix_pattern = "";
    string output_port = "";
    string pattern = "";
    string searched_port = "";
    bool not_matched;
    
    int i = 0;
    int limit = 0;
    if(bits_number == -1){
        cout << "All Possible Hop Ports are as follows: \n";
        limit = prefix_length;
    }
    else
        limit = bits_number;
    //cout << "limit = " << limit << "\n";

    while(i <= limit){
        //cout << "bit: " << i << ", " << prefix[i] << "\n";
        search_record++;
        if(node != NULL){
            pattern = node->segment;
            searched_port = node->data;
        }
        if(prefix[i] == '0')
            node = node->left;
        else
            node = node->right;

        if(node == NULL){
            break;
        }
        else{
            prefix_pattern = node->segment;
        }

        i += prefix_pattern.length() - prev_prefix_pattern.length();
        prev_prefix_pattern = prefix_pattern;

        if((node->data != "*") && (bits_number == -1)){
            if(prefix_pattern.length() == ipv4Length){
                if(prefix_pattern == prefix)
                    cout << "Prefix = " << prefix_pattern << ", " << prefix_pattern.length() << ", Hop Port = " << node->data << "\n";                    
            }
            else{
                not_matched = false;
                for(int j=0; j<prefix_pattern.length(); j++)
                    if(prefix_pattern[j] != prefix[j])
                    {
                        not_matched = true;
                    }
                if(!not_matched)
                    cout << "Prefix = " << prefix_pattern << ", " << prefix_pattern.length() << ", Hop Port = " << node->data << "\n";
            }
        }
        if(search_record > max_search_record)
            max_search_record = search_record;
        //cout << "Position = " << i << "\n";             
    }
    if(bits_number != -1){
        if(pattern.length() == ipv4Length){
            if(pattern == prefix)
                cout << "Search at Position: " << ipv4Length << ", Prefix = " << prefix_pattern << ", " << prefix_pattern.length() << ", Matched Hop Port = " << searched_port << "\n";
            else
                cout << "No Matched Hop Port\n";
        }
        else{
            cout << "Search at Position: " << bits_number << ", Prefix = " << pattern << ", " << pattern.length() << ", Hop Port = " << searched_port << "\n";
        }
    }
}

void test_routing_search(Node* root, string test_ip, string* ip_result, int bits_number)
{
    ip2binary("ip", test_ip, ip_result);
    
    string prefix = ip_result[1];
    int prefix_length = ip_result[1].length();
    //cout << prefix << ", " << prefix_length << "\n";
    if(prefix_length == 0)
    {
        prefix = ip_result[0];
        prefix_length = ip_result[0].length();
    }
    
    search_port(root, prefix_length, prefix, test_ip, bits_number);
}

void DFS_compress(Node* root)
{
    Node* node = new Node("*");
    stack<Node*> S;
    S.push(root);
    
    bool compression_routine = false;
    char connect_side = '\0';
    Node* start_node = new Node("*");
  
    while(!S.empty()){
        node = S.top();
        S.pop();

//###########################################################################
        // nonbranching termination
        // node is a branching node
        if(node->right != NULL && node->left != NULL){
            if(compression_routine){
                if(connect_side == 'R')
                    start_node->right = node;
                if(connect_side == 'L')
                    start_node->left = node;      
                //cout << start_node->parent->data << "\n";
                //cout << "End Compression: Internal\n";
            }
            compression_routine = false;
        }

        // node is a leaf
        if(node->right == NULL && node->left == NULL){
            if(compression_routine){
                if(connect_side == 'R')
                    start_node->right = node;
                if(connect_side == 'L')
                    start_node->left = node;
                //cout << start_node->parent->data << "\n";
                //cout << "End Compression: Leaf\n";
            }
            compression_routine = false;
        }
//###########################################################################        
// Ignore: TO-DO middle nonbracnhing nodes should break the bond between the end node

        if(node->right != NULL && node->left == NULL){
            if(node->data == "*" || node->data == "root"){
                if(!compression_routine){
                    //cout << "Start Compression\n";
                    compression_routine = true;
                    //start_node = node;
                    //connect_side = 'R';
                    start_node = node->parent;
                    connect_side = node->side;
                    //cout << "parent = " << start_node->data << "\n";
                }
            }
            else{ // hop on this node
                compression_routine = false;
            }
        }

        if(node->right == NULL && node->left != NULL){
            if(node->data == "*" || node->data == "root"){
                if(!compression_routine){
                    //cout << "Start Compression\n";
                    compression_routine = true;
                    //start_node = node;
                    //connect_side = 'L';                    
                    start_node = node->parent;
                    connect_side = node->side;
                    //cout << "parent = " << start_node->data << "\n";
                }
            }
            else{ // hop on this node
                compression_routine = false;
            }
        }  

//###########################################################################   

        if(!node->visited){
            node->visited = true;
            //cout << "Visit: " << node->data << "\n";
        }              

        // Right Child
        if(node->right != NULL){ 
            if(!node->right->visited)
                S.push(node->right);
        }

        // Left Child
        if(node->left != NULL){
            if(!node->left->visited)
                S.push(node->left);
        }
    }
}

void test_by_default_file(Node* root, string FileTest)
{
    ifstream infile(FileTest);
    string line;
    int cnt = 0;
    string test_ip, target_port;
    string* ip_result = new string[2];
    while(getline(infile, line))
    {
        infile >> test_ip >> target_port;
        if(infile.eof())
            break;
        cnt++;
        test_routing("file", root, test_ip, ip_result, target_port);
    }  
}

int main()
{
    //string FileInput = "prefix.txt";
    string DefaultFileInput = "routing_table.txt";
    string DefaultFileTest = "test_table.txt";

//  Build the Trie    
    string FileInput;
    cout << "Enter File Name of Your Routing Table: ";
    getline(cin, FileInput);
    if(FileInput == "default")
        FileInput = DefaultFileInput;
    cout << "Start Building the Trie\n";        
    Node* root = build(FileInput);
    DFS_compress(root);

    string ip, port, ip_binary;
    string* ip_result = new string[2];
    string test_ip;
    string target_port;
    string bits_number;
    while(true){
        NotFoundCnt = 0;
        cout << "Input an IP address (\"enter\" for exit): ";
        getline(cin, test_ip);
        if(test_ip == "")
            break;
        else if(test_ip == "default"){
            test_by_default_file(root, DefaultFileTest);
        }
        else{
            cout << "Input the hop port: ";
            getline(cin, target_port);
            if(target_port == "")
            {
                cout << "Note: press \"enter\" for listing all possible ports\n";
                cout << "Input the bit position for searching (> 0) = ";
                getline(cin, bits_number);
                if(bits_number == "")
                    test_routing_search(root, test_ip, ip_result, -1);
                else
                    test_routing_search(root, test_ip, ip_result, stoi(bits_number));
            }
            else
            {
                test_routing("solo", root, test_ip, ip_result, target_port);
            }
        }
        cout << "\n\n";
    }

    cout << "\nPath Compressed Trie Size = " << size(root) << "\n";
    cout << "Max Accesses Times =  " << max_search_record << "\n";
    return 0;
}