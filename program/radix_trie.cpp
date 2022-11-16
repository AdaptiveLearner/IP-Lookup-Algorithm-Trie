#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <bitset>
#include <ctype.h>

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

    Node(string val)
    {
        data = val;
        left = NULL;
        right = NULL;
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

void insert(Node* root, int i, int prefix_length, string prefix, string port)
{

    if(i == prefix_length)
    {
        if(root->data != "*")
            cout << "WARNING: Prefix Used; Overwrite\n";
        //cout << i << ", prefix: " << prefix << ", " << port << "\n";        
        root->data = port;
        return;
    }
    if(prefix[i] == '0')
    {
        if(root->left == NULL){
            Node* node = new Node("*");
            root->left = node;
            root = root->left;
            insert(root, i+1, prefix_length, prefix, port);            
        }
        else{
            root = root->left;
            insert(root, i+1, prefix_length, prefix, port);              
        }
    }
    else
    {
        if(root->right == NULL){
            Node* node = new Node("*");
            root->right = node;
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

void DFS(Node* node)
{
    if(node == NULL)
        return;
    DFS(node->left);
    
    cout << node->data << "\n";

    DFS(node->right);
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
            cout << "cnt = " << cnt << "\n";
            printf("%s,  %s,   %s,  %d\n", 
                 ip.c_str(), ip_result[0].c_str(), ip_result[1].c_str(), int(ip_result[1].length()));
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
    return root;
}

bool search_tree(Node* node, int prefix_length, string test_ip, string prefix, string target_port)
{
    int search_record = 0;    
    if(test_ip == root_ip)
        if(node->data == target_port){
            if(max_search_record < search_record)
                max_search_record = search_record;
            return true;
        }

    for(int i=0; i<prefix_length; i++)
    {
        //cout << "bit: " << prefix[i] << "\n";
        search_record++;
        if(prefix[i] == '0')
        {
            node = node->left;
        }
        else
        {
            node = node->right;
        }
        if(node == NULL)
        {
            return false;
        }
        //cout << i+1 << ": " <<  node->data << "\n";
        if(node->data == target_port)
        {
            if(search_record > max_search_record)
            {
                max_search_record = search_record;
            }
            return true;
        }
    }
    return false;
}

void test_routing(Node* root, string test_ip, string* ip_result, string target_port)
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
    
    if(!search_tree(root, prefix_length, test_ip, prefix, target_port))
    {
        NotFoundCnt++;
        cout << "not found #" << NotFoundCnt << ": [IP = " << test_ip << ", Hop Port = " << target_port << "]\n";
    }
}

int main()
{
    //string FileInput = "prefix.txt";
    string FileInput = "routing_table.txt";
    string FileTest = "test_table.txt";

//  Build the Trie    
    Node* root = build(FileInput);
    
   
    ifstream infile(FileTest);
    string line;
    int cnt = 0;
    string test_ip, target_port;
    string* ip_result = new string[2];
    string prefix = "";
    int prefix_length = 0;
    while(getline(infile, line))
    {
        infile >> test_ip >> target_port;
        if(infile.eof())
            break;
        cnt++;
        test_routing(root, test_ip, ip_result, target_port);
    }    

    cout << "Trie Size: " << size(root) << "\n";
    cout << "Max Search Times: " << max_search_record << "\n";
    return 0;
}