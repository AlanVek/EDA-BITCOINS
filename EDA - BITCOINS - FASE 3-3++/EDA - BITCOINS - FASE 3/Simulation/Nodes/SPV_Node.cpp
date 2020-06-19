#include "SPV_Node.h"
#include "Node/Client/AllClients.h"
#include <typeinfo>
#include "BlockChain/BlockChain.h"

namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

/*SPV_Node constructor. Uses Node constructor.*/
SPV_Node::SPV_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier, int& size, const GUIMsg& messenger) :
	Node(io_context, ip, port, identifier, size, messenger)
{
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
	actions[ConnectionType::POSTFILTER] = new POSTFilter(this);
	actions[ConnectionType::GETHEADER] = new GETHeader(this);
	actions[ConnectionType::PING] = new Ping(this);
	actions[ConnectionType::FALSETRANS] = new FalseTrans(this);
}

/*GET callback for server.*/
const std::string SPV_Node::GETResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	int neighbor = setConnectedClientID(nodeInfo);

	messenger.setMessage("Node " + std::to_string(identifier) + " is answering a request from "
		+ (neighbor == -1 ? "an unknown node." : "node " + std::to_string(neighbor)));

	json result;
	result["status"] = false;

	/*Content error.*/
	result["result"] = 2;
	return headerFormat(result.dump());
}

unsigned int generateID(unsigned char* str) {
	unsigned int ID = 0;
	int c;
	while (c = *str++)
		ID = c + (ID << 6) + (ID << 16) - ID;
	return ID;
}
/*Transforms int into hex Coded ASCII.*/
inline const std::string hexCodedASCII(unsigned int number) {
	char res[9];
	sprintf_s(res, "%08X", number);

	return res;
}

inline const std::string hash(const std::string& code) {
	return hexCodedASCII(generateID((unsigned char*)code.c_str()));
}
/*POST callback for server.*/
const std::string SPV_Node::POSTResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	int neighbor = setConnectedClientID(nodeInfo);

	messenger.setMessage("Node " + std::to_string(identifier) + " is answering a request from "
		+ (neighbor == -1 ? "an unknown node." : "node " + std::to_string(neighbor)));
	json result;
	result["status"] = true;
	result["result"] = NULL;

	/*Checks if it's a POST for merkleblock.*/
	if (request.find(MERKLEPOST) != std::string::npos) {
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of */("Data=");
		if (content == std::string::npos || data == std::string::npos)
			result["status"] = false;
		else {
			json merkle = json::parse(request.substr(data + 5, content - data - 5));

			if (!validateMerkleBlock(merkle)) {
				merkles.push_back(merkle);

				perform(ConnectionType::GETHEADER, (*std::begin(neighbors)).first, "0", NULL);
			}
		}
	}
	else {
		result["status"] = false;
		result["result"] = 2;
	}

	return headerFormat(result.dump());
}

/*Destructor. Uses Node destructor.*/
SPV_Node::~SPV_Node() {}

void SPV_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (actions.find(type) != actions.end()) {
		if (type == ConnectionType::POSTTRANS && actions[type]->isDataNull()) {
			actions[type]->setData(generateTransJSON(blockID, count, true));
		}
		else if (type == ConnectionType::FALSETRANS && actions[type]->isDataNull()) {
			actions[type]->setData(generateTransJSON(blockID, count, false));
		}

		if (!actions[type]->isDataNull()) messenger.setMessage("Node " + std::to_string(identifier) + " is performing a client request.");
		actions[type]->Perform(id, blockID, count);
		actions[type]->clearData();
	}
}

void SPV_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (actions.find(type) != actions.end()) {
		if (!actions[type]->isDataNull()) messenger.setMessage("Node " + std::to_string(identifier) + " is performing a client request.");

		actions[type]->Perform(id, blockID, key);
		actions[type]->clearData();
	}
}

/*Performs client mode. */
void SPV_Node::perform() {
	/*If request has ended...*/
	if (clients.size() && clients.front() && !clients.front()->perform()) {
		if (typeid(*clients.front()) == typeid(GETHeaderClient)) {
			const json& temp = clients.front()->getAnswer();
			if (temp.find("status") != temp.end() && temp["status"]) {
				if (temp.find("result") != temp.end()) {
					std::string res = temp["result"].dump();
					for (const auto& header : temp["result"])
						headers[header["blockid"]] = header;
				}
			}

			for (auto& merkle : merkles) {
				if (validateMerkleBlock(merkle)) {
					/*OK TRANS*/
				}
				else {
				}
			}
			merkles = json();
		}
		/*Deletes client and set pointer to null.*/
		delete clients.front();
		clients.pop_front();
	}
}

bool SPV_Node::validateMerkleBlock(const json& merkle) {
	std::string calculatedMerkleRoot = merkle["tx"][merkle["txPos"].get<int>()]["txid"];
	if (merkle.find("merklePath") == merkle.end()) {
		//print("Failed to fin merklePath");
	}
	else {
		for (auto& step : merkle["merklePath"]) {
			calculatedMerkleRoot.append(step["id"].get<std::string>());
			calculatedMerkleRoot = hash(calculatedMerkleRoot);
		}
		//print("About to check merkle");
		if (headers.find(merkle["blockid"]) != headers.end()) {
			if (headers[merkle["blockid"]]["merkleroot"] == calculatedMerkleRoot) {
				/*TRANSACTION WAS OK.*/

				//print("Transaction was OK.");

				UTXOs[merkle["tx"][merkle["txPos"].get<int>()]["txid"]] = merkle["tx"][merkle["txPos"].get<int>()];

				//print("Updated UTXO.");
				return true;
			}
			else {
				//print("Merkleroot was different.");
			}
		}
		else {
			//print("Failed to find blockid in headers.");

			return false;
		}
	}
}

const json SPV_Node::generateTransJSON(const std::string& wallet, const unsigned int amount, bool real) {
	if (!real) {
		json result, vout, vin, temp, tempVout;

		temp["txid"] = "ABCDEFGH";
		temp["outputIndex"] = 131;

		vin.push_back(temp);

		tempVout["publicid"] = wallet;
		tempVout["amount"] = amount;
		vout.push_back(tempVout);

		result["vin"] = vin;
		result["vout"] = vout;
		result["txid"] = BlockChain::calculateTXID(result);
		result["nTxin"] = 1;
		result["nTxout"] = 1;

		return result;
	}
	std::map<std::string, int> utxos;
	json result;
	int tot = 0;

	for (auto& utxo : UTXOs) {
		auto& output = utxo.second["vout"];
		for (unsigned int i = 0; i < output.size(); i++) {
			if (output[i].find("publicid") != output[i].end() && output[i]["publicid"] == publicKey) {
				tot += output[i]["amount"];
				utxos[utxo.first] = i;
			}

			if (tot >= amount) {
				json vin, vout;

				for (auto& input : utxos) {
					json temp;

					temp["txid"] = input.first;
					temp["outputIndex"] = input.second;

					vin.push_back(temp);
				}

				json tempVout;
				tempVout["publicid"] = wallet;
				tempVout["amount"] = amount;
				vout.push_back(tempVout);

				if (tot > amount) {
					tempVout["publicid"] = publicKey;
					tempVout["amount"] = tot - amount;
					vout.push_back(tempVout);
				}

				result["vin"] = vin;
				result["vout"] = vout;

				result["txid"] = BlockChain::calculateTXID(result);

				result["nTxin"] = result["vin"].size();
				result["nTxout"] = result["vout"].size();

				for (auto& tx : utxos) UTXOs.erase(tx.first);

				UTXOs[result["txid"]] = result;
				messenger.setMessage("Node " + std::to_string(identifier) + " made a transaction for " + std::to_string(amount) + " EDA coin(s)");
				return result;
			}
		}
	}

	messenger.setMessage("Node " + std::to_string(identifier) + " doesn\'t have " + std::to_string(amount) + " EDA coin(s) to perform a transaction");
	return json();
}