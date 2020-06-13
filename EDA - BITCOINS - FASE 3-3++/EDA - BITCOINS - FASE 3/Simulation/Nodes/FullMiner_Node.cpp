#include "FullMiner_Node.h"
#include "Node/Client/GETBlockClient.h"
#include <chrono>

const double timeElapsed = 30.0;

FullMiner_Node::FullMiner_Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
	const unsigned int identifier, int& size) : Full_Node(io_context, ip, port, identifier, size)
{
}

/*Performs client mode. */
void FullMiner_Node::perform() {
	static auto start = std::chrono::steady_clock::now();

	auto end = std::chrono::steady_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() > timeElapsed) {
		/*MINE BLOCK*/
		mineBlock();
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

void FullMiner_Node::mineBlock() {
	json block;
	for (const auto& trans : transactions) {
		block["tx"].push_back(trans);
	}
	block["height"] = blockChain.getBlockAmount();
	block["nTx"] = transactions.size();

	block["nonce"] = rand() % 65536;

	int blockCount;
	if ((blockCount = blockChain.getBlockAmount())) {
		char res[9];
		std::string tempID = blockChain.getBlockInfo(blockCount - 1, BlockInfo::BLOCKID);
		block["previousblockid"] = tempID;
		sprintf_s(res, "%08X", std::stoi(tempID) + 1);
		block["blockid"] = std::string(res);
	}
	else {
		block["previousblockid"] = "00000000";
		block["blockid"] = "00000001";
	}

	block["merkleroot"] = BlockChain::calculateMerkleRoot(block);

	for (auto& neighbor : neighbors) {
		actions[ConnectionType::POSTBLOCK]->setData(block);
		if (!neighbor.second.filter.length()) {
			Full_Node::perform(ConnectionType::POSTBLOCK, neighbor.first, "", NULL);
		}
	}

	blockChain.addBlock(block);

	transactions = json();
}

FullMiner_Node::~FullMiner_Node() {}