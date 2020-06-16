#include "FullMiner_Node.h"
#include "Node/Client/GETBlockClient.h"
#include <chrono>

const double timeElapsed = 15.0;
const double minerFee = 10;

FullMiner_Node::FullMiner_Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
	const unsigned int identifier, int& size) : Full_Node(io_context, ip, port, identifier, size)
{
}

/*Performs client mode. */
void FullMiner_Node::perform() {
	using namespace std::chrono;

	static auto start = steady_clock::now();

	auto end = steady_clock::now();

	if (duration_cast<seconds>(end - start).count() > timeElapsed) {
		/*MINE BLOCK*/
		mineBlock(true);
		start = end;
	}

	/*If request has ended...*/
	if (clients.size() && clients.front() && !clients.front()->perform()) {
		/*Checks if it was a GETBlock...*/
		if (typeid(*clients.front()) == typeid(GETBlockClient)) {
			/*Saves blocks.*/
			const json& temp = clients.front()->getAnswer();
			if (temp.find("status") != temp.end() && temp["status"]) {
				if (temp.find("result") != temp.end()) {
					for (const auto& block : temp["result"]) {
						blockChain.addBlock(block);
					}
				}
			}
		}
		/*Deletes client.*/
		delete clients.front();
		clients.pop_front();
	}
}

void FullMiner_Node::mineBlock(bool real) {
	json block;

	for (const auto& trans : transactions) {
		block["tx"].push_back(UTXOs[trans]);
	}

	block["tx"].push_back(getFeeTrans(real));

	block["height"] = blockChain.getBlockAmount();
	block["nTx"] = transactions.size() + 1;

	block["nonce"] = rand() % 65536;

	int blockCount;
	if ((blockCount = blockChain.getBlockAmount())) {
		block["previousblockid"] = blockChain.getBlockInfo(blockCount - 1, BlockInfo::BLOCKID);
	}
	else {
		block["previousblockid"] = "00000000";
	}

	block["merkleroot"] = BlockChain::calculateMerkleRoot(block);
	block["blockid"] = BlockChain::calculateBlockID(block);

	if (real) {
		blockChain.addBlock(block);

		transactions = json();
	}
	else {
		block["merkleroot"] = BlockChain::generateID(block["merkleroot"]);
	}

	for (auto& neighbor : neighbors) {
		actions[ConnectionType::POSTBLOCK]->setData(block);
		if (!neighbor.second.filter.length()) {
			Full_Node::perform(ConnectionType::POSTBLOCK, neighbor.first, "", NULL);
		}
	}
}

void FullMiner_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (type == ConnectionType::POSTBLOCK && actions[ConnectionType::POSTBLOCK]->isDataNull())
		actions[ConnectionType::POSTBLOCK]->setData(getBlock(blockID));

	else if (type == ConnectionType::POSTTRANS && actions[ConnectionType::POSTTRANS]->isDataNull()) {
		actions[ConnectionType::POSTTRANS]->setData(generateTransJSON(blockID, count));
	}

	else if (type == ConnectionType::FALSEBLOCK) {
		mineBlock(false);
		return;
	}

	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, count);
		actions[type]->clearData();
	}
}

const json FullMiner_Node::getFeeTrans(bool real) {
	json result;

	result["vin"];

	json vout;

	vout["publicid"] = publicKey;
	vout["amount"] = minerFee;
	result["vout"].push_back(vout);
	result["nTxin"] = 0;
	result["nTxout"] = 1;
	result["txid"] = BlockChain::generateID(std::to_string(rand()));

	if (real)
		UTXOs[result["txid"]] = result;

	return result;
}

FullMiner_Node::~FullMiner_Node() {}