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

/*GUI data.*/
/***************************************/
namespace data {
	const unsigned int width = 1000;
	const unsigned int height = 500;
	const int notSelectedIndex = -1;
	const char* autoIP = "127.0.0.1";
}
/***************************************/

/*GUI constructor. Initializes data members and sets Allegro resources.*/
GUI::GUI(void) :
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Events::NOTHING),
	state(States::INIT),
	showingConnections(true),
	showingNetworking(true),
	currentIndex(-1),
	dataIndex(-1),
	showingBlock(Shower::NOTHING),
	ID(0),
	isLocal(false),
	addingNeighbor(false)
{
	setAllegro();
	newNeighbor.type = NodeTypes::UNDEFINED;
	newNeighbor.port = 0;
	newNeighbor.ip = "";
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
bool GUI::nodeSelectionScreen() {                         /*********************************************/
	bool result = false;

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

				switch (state) {
				case States::INIT:
					result = init(&endOfSetup);
					break;
				case States::GENESIS_MODE:
					genesisConnection();
					break;
				case States::NETWORK_CREATION:
					createNetwork();
					break;
				case States::APPENDIX_MODE:
					newNode();
					break;
				case States::NODE_CREATION:
					ImGui::Text("Create node: ");
					nodes.back().ip = data::autoIP; creation(&nodes.back(), States::NODE_CONNECTION);
					if (nodes.back().ip != data::autoIP) nodes.back().ip = data::autoIP;
					break;
				case States::NODE_CONNECTION:
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

void GUI::setConnectionStr() {
	nodeConnections.clear();

	for (const auto& node : nodes) {
		nodeConnections += "Node " + std::to_string(node.index) + ":\n";
		nodeConnections += "\tType: " + ((node.type == NodeTypes::NEW_FULL) ? (std::string)"FULL\n" : (std::string)"SPV\n");
		nodeConnections += "\tConnections:\n";

		for (const auto& neighbor : node.neighbors)
			nodeConnections += "\t\t- Node " + std::to_string(nodes[neighbor].index) + " (" + ((nodes[neighbor].type == NodeTypes::NEW_FULL) ? (std::string)"FULL)\n" : (std::string)"SPV)\n");

		nodeConnections += "\n";
	}
}

bool GUI::init(bool* endOfSetup) {     /*******************************/
	bool result = false;
	/*if (nodes.size()) {
		ImGui::Text("Nodes:");
		ImGui::Text(nodeConnections.c_str());
	}*/
	if (!nodes.size()) {
		ImGui::NewLine(); ImGui::NewLine();

		/*Genensis ande Appendix mode buttons (only if no nodes were created).*/
		if (!nodes.size())
		{
			ImGui::Text("Select mode: ");
			displayWidget("Genesis Mode", [this]() { state = States::GENESIS_MODE; });
			ImGui::SameLine();
			displayWidget("Appendix Mode", [this]() { state = States::APPENDIX_MODE; });
		}

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

	/*If GUI has been initialized...*/
	else {
		/*Sets new ImGui window.*/
		newWindow("BitCoin network");

		showNetworkingInfo();
		ImGui::NewLine();

		switch (state) {
		case States::SENDER_SELECTION:
			selectSender();
			break;
		case States::RECEIVER_SELECTION:
			selectReceiver();
			break;
		case States::MESSAGE_SELECTION:
			selectMessage();
			break;
		case States::PARAM_SELECTION:
			selectParameters();
			break;
		case States::APPENDIX_MODE:
			newNode();
			break;
		case States::NODE_CREATION:
			ImGui::Text("Create node: ");
			nodes.back().ip = data::autoIP; creation(&nodes.back(), States::NODE_CONNECTION);
			if (nodes.back().ip != data::autoIP) nodes.back().ip = data::autoIP;
			break;
		case States::NODE_CONNECTION:
			connections();
			break;
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

/*Genesis mode. It creates a connection between inserted nodes from JSON file*/
//FALTA CARGAR ARCHIVO JSON PERO NO ENTENDÍ SI DEBE BUSCAR UN PATH CON EL ARCHIVO O SI UNO COMPLETA
//LA GUI Y ESO SE GUARDA EN UN JSON.
void GUI::genesisConnection() {
	static bool wrongFile = false;
	ImGui::Text("Creating peer-to-peer net");

	ImGui::Text("Enter file path: ");
	ImGui::SameLine();
	ImGui::InputText("..", &filePath);
	if (wrongFile) ImGui::Text("Wrong file path.");

	displayWidget("Done", [this]() {
		std::fstream file(filePath, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			file.close();
			wrongFile = true;
		}
		else {
			nodes.clear();
			json j = json::parse(file);
			for (const int full : j["full-nodes"]) {
				nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size(), true));
				nodes.back().port = full;
				nodes.back().ip = data::autoIP;
			}
			for (const int spv : j["spv"]) {
				nodes.push_back(NewNode(NodeTypes::NEW_SPV, nodes.size(), true));
				nodes.back().port = spv;
				nodes.back().ip = data::autoIP;
			}
			wrongFile = false;
		}
		});

	ImGui::NewLine();

	displayWidget("Create network", [this]() {state = States::NETWORK_CREATION; });
	ImGui::SameLine();
	displayWidget("Appendix Mode", [this]() { state = States::APPENDIX_MODE; });
}
void GUI::creation(NewNode* current, States nextState) {
	ImGui::Text("Enter IP:   ", ImGuiInputTextFlags_CharsDecimal);
	ImGui::SameLine();

	/*Text input for IP.*/
	ImGui::InputText("", &(current->ip));
	ImGui::Text("Enter Port: ");
	ImGui::SameLine();

	/*Int input for port.*/
	if (ImGui::InputInt("~  ", &(current->port)), 1, 5, ImGuiInputTextFlags_CharsDecimal) {
		/*Checks that port>0 or sets it to 0 otherwise.*/
		if (current->port < 0) current->port = 0;
	}

	ImGui::NewLine();

	/*'Done' button for finishing setup.*/
	displayWidget("Done", [this, &current, &nextState]() {if (current->ip.length() && current->port)
	{
		state = nextState; setConnectionStr();
	} });
}
/*Makes connections in new node.*/
void GUI::connections() {
	static bool done = false;
	static int count = 0;
	static std::vector<NewNode> neighbors(2, NewNode());
	neighbors[0].index = nodes.size();
	neighbors[1].index = nodes.size() + 1;
	if (nodes.back().type == NodeTypes::NEW_FULL) {
		ImGui::Text("Create neighbor: ");
		creation(&neighbors[0], States::EMPTYTEMP);
	}
	else {
		ImGui::Text(("Create neighbor #" + std::to_string(count + 1)).c_str());
		creation(&neighbors[count], States::EMPTYTEMP);
		if (state == States::EMPTYTEMP) {
			if (!count) {
				count++;
				state = States::NODE_CONNECTION;
			}
		}

		/*Pedir dos FULL para conectar.*/
	}

	/*Button for finishing connections setup..*/
	if (state == States::EMPTYTEMP) {
		unsigned int index = nodes.size() - 1;
		if (nodes.back().type == NodeTypes::NEW_SPV) {
			if (neighbors[0].port && neighbors[0].ip.length() && neighbors[1].port && neighbors[1].ip.length()) {
				nodes[index].neighbors.insert(nodes[index].neighbors.end(), { nodes.size(), nodes.size() + 1 });
				neighbors[0].type = NodeTypes::NEW_FULL;
				neighbors[1].type = NodeTypes::NEW_FULL;
				nodes.insert(nodes.end(), neighbors.begin(), neighbors.end());
				state = States::INIT;
				neighbors = std::vector<NewNode>(2);
				action = Events::UPDATE;
			}
		}
		else if (neighbors[0].port && neighbors[0].ip.length()) {
			nodes[index].neighbors.push_back(nodes.size());
			neighbors[0].type = NodeTypes::NEW_FULL;
			nodes.push_back(neighbors[0]);
			state = States::INIT;
			neighbors = std::vector<NewNode>(2);
			action = Events::UPDATE;
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

void GUI::showNetworkingInfo() {
	if (showingNetworking) {
		displayWidget("Hide networking info", [this]() {showingNetworking = false; });
		ImGui::Text(networkingInfo.c_str());
	}
	else
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
	const auto& neighbors = allNodes[sender]->getNeighbors();
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
	auto filter = [this]() {displayWidget("Filter (POST)", [this]() {action = Events::FILTER; state = States::PARAM_SELECTION; }); };
	auto get_blocks = [this]() {displayWidget("Block (GET)", [this]() {action = Events::GET_BLOCKS; state = States::PARAM_SELECTION; }); };
	auto get_headers = [this]() {displayWidget("Headers (GET)", [this]() {action = Events::GET_HEADERS;  state = States::PARAM_SELECTION; }); };
	auto merkleblock = [this]() {displayWidget("Merkleblock (POST)", [this]() {action = Events::MERKLEBLOCK; state = States::PARAM_SELECTION; }); };
	auto post_block = [this]() {displayWidget("Block (POST)", [this]() {action = Events::POST_BLOCK; state = States::PARAM_SELECTION; }); };
	auto transaction = [this]() {displayWidget("Transaction (POST)", [this]() {action = Events::TRANSACTION;  state = States::PARAM_SELECTION; }); };

	/*If sender is a Full Node...*/
	if (nodes[sender].type == NodeTypes::NEW_FULL) {
		/*If receiver is a Full Node...*/
		if (nodes[receiver].type == NodeTypes::NEW_FULL) {
			/*Displays allowed messages.*/
			get_blocks();  ImGui::SameLine();
			post_block();  ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}

		/*If receiver is an SPV Node, displays allowed messages.*/
		else { merkleblock(); ImGui::NewLine(); }
	}

	/*If sender is an SPV Node...*/
	else {
		/*If receiver is a Full Node...*/
		if (nodes[receiver].type == NodeTypes::NEW_FULL) {
			/*Displays allowed messages.*/
			filter(); ImGui::SameLine();
			get_headers(); ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}
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

void GUI::createNetwork() {
}

void GUI::addNeighbor(bool local, bool type) {
	if (!local) {
		ImGui::Text("Enter IP:   ", ImGuiInputTextFlags_CharsDecimal);
		ImGui::SameLine();
		ImGui::InputText("", &newNeighbor.ip);
	}

	/*Text input for IP.*/
	ImGui::Text("Enter Port: ");
	ImGui::SameLine();

	/*Int input for port.*/
	if (ImGui::InputInt("~  ", &(newNeighbor.port)), 1, 5, ImGuiInputTextFlags_CharsDecimal) {
		/*Checks that port>0 or sets it to 0 otherwise.*/
		if (newNeighbor.port < 0) newNeighbor.port = 0;
	}

	if (type) {
		ImGui::Text("Select type: ");
		ImGui::SameLine();
		displayWidget("FULL", [this]() {newNeighbor.type = NodeTypes::NEW_FULL; });
		ImGui::SameLine();
		displayWidget("SPV", [this]() {newNeighbor.type = NodeTypes::NEW_SPV; });
		if (newNeighbor.type != NodeTypes::UNDEFINED) {
			ImGui::SameLine();
			const char* text = newNeighbor.type == NodeTypes::NEW_FULL ? "FULL" : "SPV";
			ImGui::Text(text);
		}
	}

	ImGui::NewLine();

	displayWidget("Done", [this, &local]() {
		auto resetNeighbor = [this]() {addingNeighbor = false; selected = false; newNeighbor = NewNode(); };
		if (newNeighbor.port && newNeighbor.type != NodeTypes::UNDEFINED) {
			if (local) {
				for (const auto& node : allNodes) {
					if (node->getPort() == newNeighbor.port) {
						allNodes[currentIndex]->newNeighbor(node->getID(), data::autoIP, newNeighbor.port);
						allNodes[node->getID()]->newNeighbor(allNodes[currentIndex]->getID(), data::autoIP, allNodes[currentIndex]->getPort());

						resetNeighbor();
					}
				}
			}
			else {
				if (newNeighbor.ip.length()) {
					newNeighbor.index = ID;
					allNodes[currentIndex]->newNeighbor(newNeighbor.index, newNeighbor.ip, newNeighbor.port);
					nodes.push_back(newNeighbor);
					ID++;
					resetNeighbor();
				}
			}
		}
		});
}

void GUI::generalScreen() {
	showNodes();
	ImGui::NewLine();
	ImGui::NewLine();
	ImGui::Text("Actions: ");
	/*Exit button.*/
	displayWidget("Exit", [this] {action = Events::END; });
	ImGui::SameLine();
	/*New Message button.*/
	if (nodes.size() > 1)
		displayWidget("New message", [this] {state = States::SENDER_SELECTION; networkingInfo.clear(); });

	ImGui::SameLine();
	displayWidget("New node", [this]() {
		if (true) {
			state = States::APPENDIX_MODE;
		}});
}

void GUI::showNodes() {  /******************************************************/
	if (currentIndex == -1) {
		ImGui::Text("Nodes: ");
		for (const auto& node : allNodes) {
			displayWidget(("Node " + std::to_string(node->getID())).c_str(), [this, &node]() {currentIndex = node->getID(); });
			ImGui::SameLine();
		}
		//ImGui::Text(nodeConnections.c_str());
	}
	else {
		if (nodes[currentIndex].type == NodeTypes::NEW_FULL) {
			displayWidget("Add neighbor", [this]() {addingNeighbor = true; });
			if (addingNeighbor) {
				if (!selected) {
					displayWidget("Local", [this]() {isLocal = true; selected = true; });
					ImGui::SameLine();
					displayWidget("External", [this]() {isLocal = false; selected = true; });
				}
				if (selected) {
					addNeighbor(isLocal, true);
				}
			}

			/*SHOW BLOCKS*/
			showBlocks();
			displayWidget("Go Back", [this]() {currentIndex = -1; dataIndex = -1; selected = false; addingNeighbor = false; showingBlock = Shower::NOTHING; });
			if (dataIndex != -1) {
				displayActions();
			}
		}
		else {
			/*MODIFY neighborS*/
			//MEJORAR ESTO PARA CAMBIAR VARIABLE ESTATICA
			//NO FUNCIONA
			/*
			static bool checker = false;
			displayWidget(std::bind(ImGui::Checkbox, ("Modify neighbors", &checker)),
				[this]() {
					if (checker)
						showingBlock = Shower::MODIFY_neighborS;
					else
						showingBlock = Shower::NOTHING;
				});
			*/
			ImGui::SameLine();
			displayWidget("Go Back", [this]() {currentIndex = -1; dataIndex = -1; showingBlock = Shower::NOTHING; });
			if (dataIndex != -1) {
				displaySPVActions();
			}
		}
	}
}

/*Displays action buttons for SPV nodes.*/
inline void GUI::displaySPVActions() {  /***********************************************************/
/*
	NO FUNCIONA
	ImGui::NewLine();
	ImGui::Text("Action to perform: ");

	//ESTO ESTÁ MAL PUES SOLO DEBERIA MOSTRAR LOS VECINOS
	ImGui::Text("Nodes: ");
	for (const auto& node : allNodes) {
		displayWidget(("Node " + std::to_string(node->getID())).c_str(), [this, &node]() {currentIndex = node->getID(); });
		ImGui::SameLine();
	}
*/
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
	if (guiQueue)
		al_destroy_event_queue(guiQueue);
	if (guiDisp)
		al_destroy_display(guiDisp);
}

/*Displays given widget and applies callback according to widget state.*/
template <class Widget, class F1, class F2>
inline auto GUI::displayWidget(const Widget& widget, const F1& f1, const F2& f2) -> decltype(f1())
{
	if (widget())
		return f1();
	return f2();
}

/*Specialization of displayWidget template.
As ImGui::Button is the most used widget, when the given 'widget'
is actually a const char*, then the widget will be ImGui::Button.*/
template <class F1, class F2>
inline auto GUI::displayWidget(const char* txt, const F1& f1, const F2& f2)->decltype(f1()) {
	if (ImGui::Button(txt))
		return f1();
	return f2();
}

/*Getters.*/
const std::vector<GUI::NewNode>& GUI::getNodes() { return nodes; }
const unsigned int& GUI::getSenderID() { return nodes[sender].index; }
const unsigned int& GUI::getReceiverID() { return nodes[receiver].index; }
const GUI::NewNode& GUI::getNode(unsigned int index) { return nodes[index]; }
const int GUI::getAmount() { return amount; }
const std::string& GUI::getWallet() { return wallet; }

/*Sets flags to initial state.*/
void GUI::infoGotten() { wallet.clear(); amount = 0; action = Events::NOTHING; }
void GUI::updateMsg(const std::string& info) { networkingInfo.append(info); }
void GUI::setRealNodes(const std::vector<Node*>& nodes) {
	for (unsigned int i = 0; i < nodes.size(); i++) {
		if (i < allNodes.size())
			allNodes[i] = nodes[i];

		else
			allNodes.push_back(nodes[i]);
	}
}