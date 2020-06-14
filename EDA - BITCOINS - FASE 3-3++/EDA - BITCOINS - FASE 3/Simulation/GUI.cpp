#include "GUI/imgui/imgui.h"
#include "GUI/imgui/imgui_impl_allegro5.h"
#include "GUI/imgui/imgui_stdlib.h"
#include "GUI.h"
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>
#include <functional>
#include "Nodes/Node/Node.h"
#include <fstream>
#include "Nodes/FullMiner_Node.h"
#include "Nodes/SPV_Node.h"

/*GUI data.*/
/***************************************/
namespace data {
	const unsigned int width = 1000;
	const unsigned int height = 500;
	const int notSelectedIndex = -1;
	const char* autoIP = "127.0.0.1";
	const std::string word = "Creating Network";

	const char* wrongFile = "Wrong file path";
	const char* wrongFormat = "File was not JSON";
	const char* okFile = "File was OK";
}
/***************************************/

const enum class FILECODES : unsigned int {
	OKFILE,
	WRONGFILE,
	NOTJSON,
	NOTHING
};

/*GUI constructor. Initializes data members and sets Allegro resources.*/
GUI::GUI(std::vector<Node*>& nodes) :
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Events::NOTHING),
	state(States::INIT),
	currentIndex(-1),
	dataIndex(-1),
	showingBlock(Shower::NOTHING),
	allNodes(nodes)
{
	setAllegro();
}

/*Initializes Allegro resources and throws different
messages in case of different errors.*/
void GUI::setAllegro() {
	/*Initializes Allegro resources.*/
	if (!al_init())
		throw std::exception("Failed to initialize Allegro.");

	else if (!al_init_primitives_addon())
		throw std::exception("Failed to initialize primitives addon.");

	else if (!(al_install_keyboard()))
		throw std::exception("Failed to initialize keyboard addon.");

	else if (!(al_install_mouse()))
		throw std::exception("Failed to initialize mouse addon.");

	else if (!(guiQueue = al_create_event_queue()))
		throw std::exception("Failed to create event queue.");

	else if (!(guiDisp = al_create_display(data::width, data::height)))
		throw std::exception("Failed to create display.");

	else {
		/*Attaches events to event queue.*/
		al_register_event_source(guiQueue, al_get_keyboard_event_source());
		al_register_event_source(guiQueue, al_get_mouse_event_source());
		al_register_event_source(guiQueue, al_get_display_event_source(guiDisp));

		initialImGuiSetup();
	}
}
//Set up for GUI with ImGUI.
void GUI::initialImGuiSetup(void) const {
	al_set_target_backbuffer(guiDisp);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	al_set_new_display_flags(ALLEGRO_RESIZABLE);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplAllegro5_Init(guiDisp);

	//Sets screen to black.
	al_clear_to_color(al_map_rgb(0, 0, 0));
}

/*Checks if user pressed ESC or closed display.
It also deals with display resizing.*/
bool GUI::eventManager(void) {
	bool result = false;

	//Gets events.
	while ((al_get_next_event(guiQueue, &guiEvent)))
	{
		ImGui_ImplAllegro5_ProcessEvent(&guiEvent);

		/*If the display has been closed or if the user has pressed ESC, return true. */
		if (guiEvent.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (guiEvent.type == ALLEGRO_EVENT_KEY_DOWN &&
			guiEvent.keyboard.keycode == ALLEGRO_KEY_ESCAPE))
			result = true;

		//If the display has been resized, it tells ImGUI to take care of it.
		else if (guiEvent.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			al_acknowledge_resize(guiDisp);
			ImGui_ImplAllegro5_CreateDeviceObjects();
		}
	}
	return result;
}

/*Initial data input for node selection.
Returns true if setup was done successfully or
false if user asked to leave.*/
bool GUI::nodeSelectionScreen(bool* newNet) {                         /*********************************************/
	bool result = false;
	*newNet = false;
	bool endOfSetup = false;
	al_set_target_backbuffer(guiDisp);

	/*If GUI is still in initializing state...*/
	if (state < States::INIT_DONE) {
		/*While setup isn't finished...*/
		while (!endOfSetup) {
			/*If user asked to leave, breaks loop.*/
			if (eventManager()) { endOfSetup = true; }

			else {
				/*New ImGui window.*/
				newWindow("Initial Setup");
				ImGui::Text("Initial Setup: ");
				ImGui::NewLine();

				/*Performs action according to state.*/
				switch (state) {
				case States::INIT:
					/*Initial screen.*/
					result = init(&endOfSetup);
					break;
				case States::GENESIS_MODE:
					/*Opens file and creates nodes.*/
					genesisConnection();
					break;
				case States::NETWORK_CREATION:

					/*Exits and tells simulation to create network
					using algorithm.*/
					result = true;
					*newNet = true;
					endOfSetup = true;
					addMiner();
					break;
				case States::APPENDIX_MODE:
					/*Adds new node.*/
					newNode();
					break;
				case States::NODE_CREATION:

					/*Requests data for latest node created.*/
					ImGui::Text("Create node: ");
					nodes.back().ip = data::autoIP; creation(&nodes.back(), States::NODE_CONNECTION);
					if (nodes.back().ip != data::autoIP) nodes.back().ip = data::autoIP;
					break;
				case States::NODE_CONNECTION:
					/*Connects latest node with other nodes from an existint network.*/
					connections();
					break;
				default:
					break;
				}
				/*Rendering.*/
				ImGui::End();
				render();
			}
		}
	}

	return result;
}

/*Initial screen.*/
bool GUI::init(bool* endOfSetup) {     /*******************************/
	bool result = false;

	/*If user already created nodes, it exits.*/
	if (!nodes.size()) {
		ImGui::NewLine(); ImGui::NewLine();

		/*Mode selection.*/
		ImGui::Text("Select mode: ");
		displayWidget("Genesis Mode", [this]() { state = States::GENESIS_MODE; });
		ImGui::SameLine();
		displayWidget("Appendix Mode", [this]() { state = States::APPENDIX_MODE; });

		ImGui::NewLine(); ImGui::NewLine();

		/*Exit button.*/
		displayWidget("Exit", [&result]() {result = true; });
		ImGui::SameLine();

		/*Go button. Breaks setup loop.*/
		displayWidget("Go", [&endOfSetup, this, &result]() {
			if (nodes.size()) { *endOfSetup = true; result = true; state = States::INIT_DONE; }});
	}
	else {
		*endOfSetup = true;
		result = true;
		state = States::INIT_DONE;
	}
	return result;
}

//Cycle that shows menu (called with every iteration).
Events GUI::checkStatus(void) {
	Events result = Events::NOTHING;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns Events::END.
	if (eventManager())
		result = Events::END;

	else {
		/*Sets new ImGui window.*/
		newWindow("BitCoin network");

		/*Shows networking info.*/
		showNetworkingInfo();
		ImGui::NewLine();

		static int cantDots = 0;

		/*Performs action according to state.*/
		switch (state) {
			/*Loading screen while program is creating network.*/
		case States::NETWORK_CREATION:
			/*FOR NOW, HARDCODED DOTS APPEARING.*/
			if (cantDots >= 1000) cantDots = 0;
			ImGui::Text((data::word + std::string(cantDots / 300, '.')).c_str());
			cantDots++;
			result = Events::KEEPCREATING;
			break;

			/*Selects sender of new message.*/
		case States::SENDER_SELECTION:
			selectSender();
			break;

			/*Selects receiver of new message.*/
		case States::RECEIVER_SELECTION:
			selectReceiver();
			break;

			/*Selects type of new message.*/
		case States::MESSAGE_SELECTION:
			selectMessage();
			break;

			/*Selects content of new message.*/
		case States::PARAM_SELECTION:
			selectParameters();
			break;

			/*Adds new node.*/
		case States::APPENDIX_MODE:
			newNode();
			break;

			/*Requests data for latest node created.*/
		case States::NODE_CREATION:
			ImGui::Text("Create node: ");
			nodes.back().ip = data::autoIP; creation(&nodes.back(), States::NODE_CONNECTION);
			if (nodes.back().ip != data::autoIP) nodes.back().ip = data::autoIP;
			break;

			/*Connects latest node with other nodes from an existint network.*/
		case States::NODE_CONNECTION:
			connections();
			result = action;
			action = Events::NOTHING;
			break;

			/*General screen with GO, EXIT, NEWNODE and NEWMESSAGE buttons.*/
		default:
			generalScreen();
			result = action;
			action = Events::NOTHING;
		}
		/*Rendering.*/
		ImGui::End();
		render();
	}
	return result;
}

/*New node type selection.*/
void GUI::newNode() {             /***************************************************/
	ImGui::Text("Select type: ");
	ImGui::SameLine();

	/*SPV button.*/
	displayWidget("SPV", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_SPV, nodes.size(), true)); state = States::NODE_CREATION; });
	ImGui::SameLine();

	/*FULL button.*/
	displayWidget("FULL", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size(), true)); state = States::NODE_CREATION; });
}

/*Genesis mode. Loads JSON file, and creates nodes accordingly. */
void GUI::genesisConnection() {
	/*Sets flag for error checking.*/
	static FILECODES fileOK = FILECODES::NOTHING;
	static std::string filePath = "test.json";
	ImGui::Text("Creating peer-to-peer net");

	/*Path for file.*/
	ImGui::Text("Enter file path: ");
	ImGui::SameLine();
	ImGui::InputText("..", &filePath);

	/*Prints message according to validity of path.*/
	switch (fileOK) {
	case FILECODES::WRONGFILE:
		ImGui::Text(data::wrongFile);
		break;
	case FILECODES::NOTJSON:
		ImGui::Text(data::wrongFormat);
		break;
	case FILECODES::OKFILE:
		ImGui::Text(data::okFile);
		break;
	default:
		break;
	}

	static json j;

	/*Load File button.*/
	displayWidget("Load File", [this]() {
		/*Attempts to open file or sets flag to WRONG if unable.*/
		std::fstream file(filePath, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			file.close();
			fileOK = FILECODES::WRONGFILE;
		}
		else {
			/*Clears node vector.*/
			nodes.clear();
			try {
				/*Attempts to read JSON file.*/
				j = json::parse(file);

				/*Loads full nodes.*/
				for (const int full : j["full-nodes"]) {
					nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size(), true));
					nodes.back().port = full;
					nodes.back().ip = data::autoIP;
				}

				/*Loads SPV nodes.*/
				for (const int spv : j["spv"]) {
					nodes.push_back(NewNode(NodeTypes::NEW_SPV, nodes.size(), true));
					nodes.back().port = spv;
					nodes.back().ip = data::autoIP;
				}

				/*Sets flag to OK.*/
				fileOK = FILECODES::OKFILE;
			}

			/*If file wasn't JSON, sets flag to WRONG_FORMAT.*/
			catch (std::exception&) {
				fileOK = FILECODES::NOTJSON;
			}

			file.close();
		}
		});

	ImGui::NewLine();

	/*Button to create network.*/
	displayWidget("Create network", [this]() {if (j.size()) {
		filePath.clear(); state = States::NETWORK_CREATION;
		fileOK = FILECODES::NOTHING;
		j.clear();
	}});
	ImGui::SameLine();

	/*Button to switch to appendix mode.*/
	displayWidget("Appendix Mode", [this]() { filePath.clear(); state = States::APPENDIX_MODE; j.clear(); });
}

/*Creates a node.*/
void GUI::creation(NewNode* current, States nextState) {
	/*IP input.*/
	ImGui::Text("Enter IP:   ", ImGuiInputTextFlags_CharsDecimal);
	ImGui::SameLine();
	ImGui::InputText("", &(current->ip));

	/*Port input.*/
	ImGui::Text("Enter Port: ");
	ImGui::SameLine();
	if (ImGui::InputInt("~  ", &(current->port)), 1, 5, ImGuiInputTextFlags_CharsDecimal) {
		/*Checks that port>0 or sets it to 0 otherwise.*/
		if (current->port < 0) current->port = 0;
	}

	ImGui::NewLine();

	/*'Done' button for finishing setup.*/
	displayWidget("Done", [this, &current, &nextState]() {if (current->ip.length() && current->port)
	{
		state = nextState;
	}});
}
/*Makes connections in new node.*/
void GUI::connections() {
	static bool done = false;
	static int count = 0;
	static std::vector<NewNode> neighbors(2, NewNode());
	neighbors[0].index = nodes.size();
	neighbors[1].index = nodes.size() + 1;

	/*If node is FULL node, set one new neighbor.*/
	if (nodes.back().type == NodeTypes::NEW_FULL) {
		ImGui::Text("Create neighbor: ");
		creation(&neighbors[0], States::EMPTYTEMP);
	}
	else {
		/*If it's SPV, set two new neighbors.*/
		ImGui::Text(("Create neighbor #" + std::to_string(count + 1)).c_str());
		creation(&neighbors[count], States::EMPTYTEMP);
		if (state == States::EMPTYTEMP) {
			if (!count) {
				count++;
				state = States::NODE_CONNECTION;
			}
		}
	}

	/*Checks if setup finished correctly.*/
	if (state == States::EMPTYTEMP) {
		unsigned int index = nodes.size() - 1;
		bool checker = true;

		/*If it's SPV...*/
		if (nodes.back().type == NodeTypes::NEW_SPV) {
			/*Checks validity of neighbors' data.*/
			for (const auto& node : neighbors) { if (!node.ip.length() || !node.port) checker = false; }
			if (checker) {
				/*Inserts neighbors to new node's neighbors.*/
				nodes[index].neighbors.insert(nodes[index].neighbors.end(), { nodes.size(), nodes.size() + 1 });

				/*Sets neighbor's types to FULL.*/
				for (auto& node : neighbors) node.type = NodeTypes::NEW_FULL;

				/*Inserts neighbors in nodes vector.*/
				nodes.insert(nodes.end(), neighbors.begin(), neighbors.end());

				/*Sets flags and resets neighbors vector.*/
				state = States::INIT;
				neighbors = std::vector<NewNode>(2);
				action = Events::UPDATE;
				count = 0;
			}
		}

		/*If it's a FULL, first checks validity of neighbor's data.*/
		else if (neighbors[0].port && neighbors[0].ip.length()) {
			/*Inserts neighbor to new node's neighbors.*/
			nodes[index].neighbors.push_back(nodes.size());

			/*Sets neighbor's type to FULL.*/
			neighbors[0].type = NodeTypes::NEW_FULL;

			/*Inserts neighbor in nodes vector.*/
			nodes.push_back(neighbors[0]);

			/*Sets flags and resets neighbors vector.*/
			state = States::INIT;
			neighbors = std::vector<NewNode>(2);
			action = Events::UPDATE;
			count = 0;
		}
	};
}

/*Shows node's connections.*/
void GUI::showConnections() {
	ImGui::NewLine(); ImGui::NewLine();
	ImGui::Text("Connected with: ");
	ImGui::SameLine();
	if (nodes.back().neighbors.size()) {
		/*Loops through every node in the current node's 'neighbors' vector.*/
		for (const auto& neighbor : nodes.back().neighbors) {
			/*Shows the node's index.*/
			ImGui::Text(("Node " + std::to_string(neighbor)).c_str());
			ImGui::SameLine();
		}
	}
	else
		ImGui::Text("None");
}
/*Shows networking info.*/
void GUI::showNetworkingInfo() {
	static bool showingNetworking = true;
	if (showingNetworking) {
		/*Shows info if button is activated.*/
		displayWidget("Hide networking info", [this]() {showingNetworking = false; });
		ImGui::Text(networkingInfo.c_str());
	}
	else

		/*Sets button for showing info if it's desactivated.*/
		displayWidget("Show networking info", [this]() {showingNetworking = true; });
}

/*Sender selection.*/
void GUI::selectSender() {
	ImGui::Text("Select Sender: ");

	/*Loops through every node...*/
	for (const auto& node : allNodes) {
		/*Sets a button with the node's index.*/
		displayWidget(("Node " + std::to_string(node->getID())).c_str(),
			[this, &node]() {sender = node->getID(); state = States::RECEIVER_SELECTION; });
		ImGui::SameLine();
	}
}
/*Receiver selection.*/
void GUI::selectReceiver() {
	int index;
	for (int i = 0; i < allNodes.size(); i++) {
		if (allNodes[i]->getID() == sender)
			index = i;
	}

	/*Gets node's neighbors.*/
	const auto& neighbors = allNodes[index]->getNeighbors();

	/*Adjusts neighbors if node has new neighbors set from outside of GUI.*/
	if (neighbors.size() > nodes[sender].neighbors.size()) {
		for (unsigned int i = nodes[sender].neighbors.size(); i < neighbors.size(); i++) {
			nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size(), false));
			nodes[sender].neighbors.push_back(nodes.back().index);
		}
	}
	if (neighbors.size()) {
		ImGui::Text("Select Receiver: ");

		/*Loops through every node within the sender's neighbors.*/
		for (const auto& neighbor : neighbors) {
			/*Sets a button with the node's index.*/
			displayWidget(("Node " + std::to_string(neighbor.first)).c_str(),
				[this, &neighbor]() {receiver = neighbor.first; state = States::MESSAGE_SELECTION; });
			ImGui::SameLine();
		}
	}
	else
		state = States::INIT_DONE;
}

/*Message selection.*/
void GUI::selectMessage() {
	action = Events::NOTHING;
	ImGui::Text("Select Message Type: ");
	ImGui::NewLine();

	/*Sets button setters for every type of message.*/
	/*auto filter = [this]() {displayWidget("Filter (POST)", [this]() {action = Events::FILTER; state = States::PARAM_SELECTION; }); };
	auto get_blocks = [this]() {displayWidget("Block (GET)", [this]() {action = Events::GET_BLOCKS; state = States::PARAM_SELECTION; }); };
	auto get_headers = [this]() {displayWidget("Headers (GET)", [this]() {action = Events::GET_HEADERS;  state = States::PARAM_SELECTION; }); };
	auto merkleblock = [this]() {displayWidget("Merkleblock (POST)", [this]() {action = Events::MERKLEBLOCK; state = States::PARAM_SELECTION; }); };
	auto post_block = [this]() {displayWidget("Block (POST)", [this]() {action = Events::POST_BLOCK; state = States::PARAM_SELECTION; }); };
	*/auto transaction = [this]() {displayWidget("Transaction (POST)", [this]() {action = Events::TRANSACTION;  state = States::PARAM_SELECTION; }); };

	/*If sender is a Full Node...*/
	if (nodes[sender].type != NodeTypes::NEW_SPV) {
		/*If receiver is a Full Node...*/
		if (nodes[receiver].type != NodeTypes::NEW_SPV) {
			///*Displays allowed messages.*/
			//get_blocks();  ImGui::SameLine();
			//post_block();  ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}
		else
			state = States::INIT_DONE;

		///*If receiver is an SPV Node, displays allowed messages.*/
		//else { merkleblock(); ImGui::NewLine(); }
	}

	/*If sender is an SPV Node...*/
	else {
		/*If receiver is a Full Node...*/
		if (nodes[receiver].type != NodeTypes::NEW_SPV) {
			///*Displays allowed messages.*/
			//filter(); ImGui::SameLine();
			//get_headers(); ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}
		else
			state = States::INIT_DONE;
	}
}

void GUI::selectParameters() {
	switch (action) {
		/*Filter (POST).*/
	case Events::FILTER:
		state = States::INIT_DONE;
		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		state = States::INIT_DONE;
		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		state = States::INIT_DONE;
		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		state = States::INIT_DONE;
		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		state = States::INIT_DONE;
		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		ImGui::Text("Enter amount:        ");
		ImGui::SameLine();
		ImGui::InputInt("..", &amount);
		ImGui::Text("Enter public wallet: ");
		ImGui::SameLine();
		ImGui::InputText("_.", &wallet);
		ImGui::NewLine();
		displayWidget("Done", [this]() {if (wallet.length() && amount) { state = States::INIT_DONE; } });
		break;
	default:
		break;
	}
}

/*Adds new neighbor to node.*/
bool GUI::addNeighbor(bool local, bool type) {
	static NewNode neighbor;

	/*Sets new neighbor's IP and port.*/
	creation(&neighbor, States::EMPTYTEMP);
	if (local && neighbor.ip != data::autoIP) neighbor.ip = data::autoIP;

	ImGui::NewLine();

	/*"Done" button.*/
	if (state == States::EMPTYTEMP) {
		/*If port was OK...*/
		if (neighbor.port) {
			/*If it's a local neighbor...*/
			if (local) {
				/*Searches for node's ID and sets both new neighbors.*/
				for (const auto& node : allNodes) {
					if (node->getPort() == neighbor.port) {
						allNodes[currentIndex]->perform(ConnectionType::PING, NULL, node->getIP(), node->getPort());
						allNodes[currentIndex]->newNeighbor(node->getID(), node->getIP(), node->getPort(), node->getWallet());
						node->newNeighbor(allNodes[currentIndex]->getID(), data::autoIP, allNodes[currentIndex]->getPort(), node->getWallet());
					}
				}
				neighbor = NewNode();
				return true;
			}

			/*If it's not local...*/
			else {
				/*Checks if IP was OK.*/
				if (neighbor.ip.length()) {
					/*Sets new neighbor and saves node in nodes vector.*/
					neighbor.index = nodes.size();
					neighbor.type = NodeTypes::NEW_FULL;
					allNodes[currentIndex]->newNeighbor(neighbor.index, neighbor.ip, neighbor.port, "");
					allNodes[currentIndex]->perform(ConnectionType::PING, NULL, neighbor.ip, neighbor.port);
					nodes.push_back(neighbor);

					neighbor = NewNode();
					return true;
				}
			}
		}
	}
	else
		return false;
}

/*General screen after GUI has been initialized.*/
void GUI::generalScreen() {
	/*Shows nodes.*/
	showNodes();
	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::Text("Actions: ");

	/*Exit button.*/
	displayWidget("Exit", [this] {action = Events::END; });
	ImGui::SameLine();

	/*New message button.*/
	if (nodes.size() > 1)
		displayWidget("New message", [this] {state = States::SENDER_SELECTION; networkingInfo.clear(); });

	/*New mode button.*/
	ImGui::SameLine();
	displayWidget("New node", [this]() {state = States::APPENDIX_MODE; });
}

/*Shows nodes.*/
void GUI::showNodes() {
	static bool doingNeighbors = false;
	static bool isLocal = false;
	static bool selected = false;
	static int neighborIndex = -1;

	if (currentIndex == -1) {
		ImGui::Text("Nodes: ");

		/*Displays buttons with nodes' IDs.*/
		for (int i = 0; i < allNodes.size(); i++) {
			displayWidget(("Node " + std::to_string(allNodes[i]->getID())).c_str(), [this, i]() {currentIndex = i; });
			ImGui::SameLine();
		}
	}

	/*If a specific node has been selected...*/
	else {
		bool checker = typeid(*allNodes[currentIndex]) != typeid(SPV_Node);
		const std::string typeText = !checker ? "SPV" : (typeid(*allNodes[currentIndex]) == typeid(Full_Node) ? "FULL" : "FULL MINER");
		ImGui::Text(("Type: " + typeText).c_str());
		ImGui::Text("Neighbors: ");
		ImGui::SameLine();
		for (auto& neighbor : allNodes[currentIndex]->getNeighbors()) {
			ImGui::Text(("Node " + std::to_string(neighbor.first)).c_str());
			ImGui::SameLine();
		}
		ImGui::NewLine();
		/*If it's a full node...*/
		if (checker) {
			neighborButtons(doingNeighbors, isLocal, selected);

			/*Shows blocks*/
			showBlocks();

			//displayActions();
		}

		/*If it was SPV...*/
		else {
			/*displayWidget("Modify Neighbors", []() {doingNeighbors = true; });
			if (doingNeighbors) {
				if (neighborIndex == -1) {
					for (auto& neighbor : allNodes[currentIndex]->getNeighbors()) {
						displayWidget(("Node " + std::to_string(neighbor.first)).c_str(), [&neighbor]() {neighborIndex = neighbor.first; });
					}
				}
				else {
					if (modifyNeighbor(neighborIndex)) {
						neighborIndex = -1;
					};
				}
			}*/
		}

		ImGui::SameLine();
		/*Button to go back.*/
		displayWidget("Go Back", [this]() {currentIndex = -1; dataIndex = -1; selected = false; doingNeighbors = false; showingBlock = Shower::NOTHING; });
		if (dataIndex != -1) {
			displayActions();
		}
	}
}

void GUI::addMiner() {
	bool used = false;
	int minerPort;

	minerPort = rand() % 1000;
	if (abs((int)nodes[0].port - minerPort) < 10) { used = true; }
	while (used) {
		used = false;
		minerPort = rand() % 1000;
		for (auto& node : nodes) {
			if (abs((int)node.port - minerPort) < 5) { used = true; }
		}
	}

	nodes.push_back(NewNode(NodeTypes::NEW_MINER, nodes.size(), true));
	nodes.back().port = minerPort;
	nodes.back().ip = "127.0.0.1";
}

/*Displays action buttons for SPV nodes.*/
bool GUI::modifyNeighbor(int id) {  /***********************************************************/
	/*static NewNode temp;
	temp.index = allNodes[currentIndex]->getID();
	temp.ip = allNodes[currentIndex]->getIP();
	temp.port = allNodes[currentIndex]->getPort();

	States whichState = state;

	creation(&temp, States::EMPTYTEMP);

	if (state == States::EMPTYTEMP) {
		state = whichState;
		allNodes[currentIndex]->newNeighbor(id, temp.ip, temp.port);
		nodes[currentIndex].neighbors
	}*/

	return true;
}

void GUI::neighborButtons(bool& addingNeighbor, bool& isLocal, bool& selected) {
	/*Button to add new neighbor.*/
	displayWidget("Add neighbor", [this, &addingNeighbor]() {addingNeighbor = true; });

	/*If user asked to set a new neighbor...*/
	if (addingNeighbor) {
		/*Sets if neighbor is local or external.*/
		if (!selected) {
			displayWidget("Local", [this, &isLocal, &selected]() {isLocal = true; selected = true; });
			ImGui::SameLine();
			displayWidget("External", [this, &isLocal, &selected]() {isLocal = false; selected = true; });
		}
		if (selected) {
			/*Once selected, adds the neighbor.*/
			if (addNeighbor(isLocal, true)) {
				addingNeighbor = false;
				selected = false;
				isLocal = false;
			}
		}
	}
}

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::NewLine();
	ImGui::Text("Action to perform: ");

	/*Creates buttons for different functionalities.*/
	displayWidget("Block ID", ([this]() {showingBlock = Shower::BLOCKID; }));
	ImGui::SameLine();
	displayWidget("Previous ID", ([this]() {showingBlock = Shower::PREVIOUS_BLOCKID; }));
	ImGui::SameLine();
	displayWidget("nTx", ([this]() {showingBlock = Shower::NTX; }));
	ImGui::SameLine();
	displayWidget("Block Number", ([this]() {showingBlock = Shower::BLOCK_NUMBER; }));
	ImGui::SameLine();
	displayWidget("Nonce", ([this]() {showingBlock = Shower::NONCE; }));
	ImGui::SameLine();
	displayWidget("Calculate MR", ([this]() {showingBlock = Shower::SEE_MROOT; }));
	ImGui::SameLine();
	displayWidget("Validate MR", ([this]() {showingBlock = Shower::VALIDATE_MROOT; }));
	ImGui::SameLine();
	displayWidget("Print tree", ([this]() {showingBlock = Shower::PRINT_TREE; }));

	/*Acts upon action selected.*/
	switch (showingBlock) {
	case Shower::BLOCKID:
		ImGui::Text(allNodes[currentIndex]->getData()[dataIndex]["blockid"].get<std::string>().c_str());
		break;
	case Shower::VALIDATE_MROOT:
		ImGui::Text(allNodes[currentIndex]->validateMRoot(dataIndex).c_str());
		break;
	case Shower::BLOCK_NUMBER:
		ImGui::Text(std::to_string(allNodes[currentIndex]->getData()[dataIndex]["height"].get<int>()).c_str());
		break;
	case Shower::NONCE:
		ImGui::Text(std::to_string(allNodes[currentIndex]->getData()[dataIndex]["nonce"].get<int>()).c_str());
		break;
	case Shower::NTX:
		ImGui::Text(std::to_string(allNodes[currentIndex]->getData()[dataIndex]["nTx"].get<int>()).c_str());
		break;
	case Shower::PREVIOUS_BLOCKID:
		ImGui::Text(allNodes[currentIndex]->getData()[dataIndex]["previousblockid"].get<std::string>().c_str());
		break;
	case Shower::PRINT_TREE:
		ImGui::Text(allNodes[currentIndex]->printTree(dataIndex).c_str());
		break;
	case Shower::SEE_MROOT:
		ImGui::Text(allNodes[currentIndex]->getData()[dataIndex]["merkleroot"].get<std::string>().c_str());
		break;
	}
}

/*For every block in the vector, it shows it.*/
void GUI::showBlocks(void) {
	bool checker;

	/*Sets a button with the blocks number to determine whether the user wants to see each block's data.*/
	for (unsigned int i = 0; i < allNodes[currentIndex]->getData().size(); i++) {
		checker = (dataIndex == i);
		displayWidget(std::bind(ImGui::Checkbox, ("Block " + std::to_string(i)).c_str(), &checker),

			[this, i, &checker]() {
				if (checker) dataIndex = i;
				else dataIndex = -1;
			});
		ImGui::SameLine();
	}
}
/*Sets a new ImGUI frame and window.*/
inline void GUI::newWindow(const char* title) const {
	//Sets new ImGUI frame.
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	//Sets new window positioned at (0,0).
	ImGui::SetNextWindowSize(ImVec2(data::width, data::height));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	/*Begins window and sets title.*/
	ImGui::Begin(title);
}

/*Rendering.*/
inline void GUI::render() const {
	ImGui::Render();
	al_clear_to_color(al_map_rgb(0, 0, 0));
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();
}

/*Cleanup. Frees resources.*/
GUI::~GUI() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	if (guiQueue) al_destroy_event_queue(guiQueue);
	if (guiDisp) al_destroy_display(guiDisp);
}

/*Displays given widget and applies callback according to widget state.*/
template <class Widget, class F1, class F2>
inline auto GUI::displayWidget(const Widget& widget, const F1& f1, const F2& f2) -> decltype(f1())
{
	if (widget()) return f1();
	return f2();
}

/*Specialization of displayWidget template.
As ImGui::Button is the most used widget, when the given 'widget'
is actually a const char*, then the widget will be ImGui::Button.*/
template <class F1, class F2>
inline auto GUI::displayWidget(const char* txt, const F1& f1, const F2& f2)->decltype(f1()) {
	if (ImGui::Button(txt)) return f1();
	return f2();
}

/*Getters.*/
const std::vector<GUI::NewNode>& GUI::getNodes() { return nodes; }
const unsigned int& GUI::getSenderID() { return nodes[sender].index; }
const unsigned int& GUI::getReceiverID() { return nodes[receiver].index; }
const GUI::NewNode& GUI::getNode(unsigned int index) { return nodes[index]; }
const int GUI::getAmount() { return amount; }
const std::string& GUI::getWallet() { return wallet; }

/*Setters*/
void GUI::networkDone() { state = States::INIT_DONE; }
void GUI::infoGotten() { wallet.clear(); amount = 0; action = Events::NOTHING; }
void GUI::updateMsg(const std::string& info) { networkingInfo.append(info); }