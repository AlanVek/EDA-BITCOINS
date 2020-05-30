#include "imgui/imgui.h"
#include "imgui/imgui_impl_allegro5.h"
#include "imgui/imgui_stdlib.h"
#include "GUI.h"
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>
#include <functional>

/*GUI data.*/
/***************************************/
namespace data {
	const unsigned int width = 1000;
	const unsigned int height = 500;
}
/***************************************/

/*GUI constructor. Initializes data members and sets Allegro resources.*/
GUI::GUI(void) :
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Events::NOTHING),
	state(States::INIT)
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
bool GUI::nodeSelectionScreen() {
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
				newWindow();
				ImGui::Text("Initial Setup: ");
				ImGui::NewLine();

				if (state == States::INIT) {
					/*New Node button.*/
					displayWidget("New Node", [this]() { state = States::NODE_SELECTION; });

					ImGui::NewLine(); ImGui::NewLine();

					/*Exit button.*/
					displayWidget("Exit", [&endOfSetup]() {endOfSetup = true; });
					ImGui::SameLine();

					/*Go button. Breaks setup loop.*/
					displayWidget("Go", [&endOfSetup, this, &result]() {
						if (nodes.size()) { endOfSetup = true; result = true; state = States::INIT_DONE; }});
				}

				/*Select node type.*/
				else if (state == States::NODE_SELECTION) { newNode(); }

				/*Select neighbors.*/
				else if (state == States::NODE_CONNECTION) { connections(); }

				/*Select IP and port.*/
				else if (state == States::NODE_CREATION) { creation(); }

				/*Rendering.*/
				ImGui::End();
				render();
			}
		}
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
	else if (state >= States::INIT_DONE) {
		/*Sets new ImGui window.*/
		newWindow();

		ImGui::Text("Actions: ");

		ImGui::NewLine();

		ImGui::NewLine();

		/*Sender selection.*/
		if (state == States::SENDER_SELECTION) selectSender();

		/*Receiver selection.*/
		else if (state == States::RECEIVER_SELECTION) selectReceiver();

		/*Message selection.*/
		else if (state == States::MESSAGE_SELECTION) { selectMessage(); }

		/*Parameter(s) selection.*/
		else if (state == States::PARAM_SELECTION) { selectParameters(); }
		else {
			/*Exit button.*/
			displayWidget("Exit", [&result] {result = Events::END; });

			ImGui::SameLine();

			/*New Message button.*/
			displayWidget("New message", [this] {state = States::SENDER_SELECTION; });

			if ((bool)action) result = action;
		}

		/*Rendering.*/
		ImGui::End();
		render();
	}
	return result;
}

/*New node type selection.*/
void GUI::newNode() {
	ImGui::Text("Select type: ");
	ImGui::SameLine();

	/*SVP button.*/
	displayWidget("SVP", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_SVP, nodes.size())); state = States::NODE_CONNECTION; });
	ImGui::SameLine();

	/*FULL button.*/
	displayWidget("FULL", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size())); state = States::NODE_CONNECTION; });
}

/*Makes connections in new node.*/
void GUI::connections() {
	/*Validates node (no repeated neighbors).*/
	auto nodeValidation = [this](unsigned int index) {
		for (const auto& id : nodes.back().neighbors)
			if (nodes[id].index == index)
				return false;
		return true;
	};

	ImGui::NewLine();
	ImGui::Text("Choose connected nodes: ");

	ImGui::Text("******************************************");
	/*For every node available...*/
	for (unsigned int i = 0; i < nodes.size() - 1; i++) {
		/*If at least one is FULL (no SVP-SVP neighbors)...*/
		if (nodes[i].type == NodeTypes::NEW_FULL || nodes.back().type == NodeTypes::NEW_FULL) {
			/*Sets a button with the node's index.*/
			displayWidget(
				("Node " + std::to_string(i)).c_str(),

				/*If pressed, it sets the new neighbor in each of the nodes' 'neighbors' vector.*/
				[this, i, &nodeValidation]() {
					if (nodeValidation(nodes[i].index)) {
						nodes.back().neighbors.push_back(i);
						nodes[i].neighbors.push_back(nodes.size() - 1);
					}
				}
			);
			ImGui::SameLine();
		}
	}
	ImGui::NewLine();
	ImGui::Text("******************************************");

	/*Shows node's current connections.*/
	showConnections();

	/*Button for finishing connections setup..*/
	ImGui::NewLine(); ImGui::NewLine();
	displayWidget("Done", [this]() {state = States::NODE_CREATION; });
}

/*Shows node's connections.*/
void GUI::showConnections() {
	ImGui::NewLine(); ImGui::NewLine();
	ImGui::Text("Connected with: ");
	ImGui::SameLine();

	/*Loops through every node in the current node's 'neighbors' vector.*/
	for (const auto& neighbor : nodes.back().neighbors) {
		/*Shows the node's index.*/
		ImGui::Text(("Node " + std::to_string(neighbor)).c_str());
		ImGui::SameLine();
	}
}

/*Sender selection.*/
void GUI::selectSender() {
	ImGui::Text("Select Sender: ");

	/*Loops through every node...*/
	for (const auto& node : nodes) {
		/*Sets a button with the node's index.*/
		displayWidget(("Node " + std::to_string(node.index)).c_str(),
			[this, &node]() {sender = node.index; state = States::RECEIVER_SELECTION; });
		ImGui::SameLine();
	}
}
/*Receiver selection.*/
void GUI::selectReceiver() {
	ImGui::Text("Select Receiver: ");

	/*Loops through every node within the sender's neighbors.*/
	for (const auto& neighbor : nodes[sender].neighbors) {
		/*Sets a button with the node's index.*/
		displayWidget(("Node " + std::to_string(neighbor)).c_str(),
			[this, &neighbor]() {receiver = neighbor; state = States::MESSAGE_SELECTION; });
		ImGui::SameLine();
	}
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

		/*If receiver is an SVP Node, displays allowed messages.*/
		else { merkleblock(); ImGui::NewLine(); }
	}

	/*If sender is an SVP Node...*/
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
	bool wildcard = false;

	switch (action) {
		/*Filter (POST).*/
	case Events::FILTER:
		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		ImGui::Text("Enter Block ID: ");
		ImGui::SameLine();
		ImGui::InputText("_", &blockID);
		ImGui::Text("Enter count:    ");
		ImGui::SameLine();
		if (ImGui::InputInt(".", &count)) { if (count < 0) count = 0; }
		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		ImGui::Text("Enter Block ID: ");
		ImGui::SameLine();
		ImGui::InputText("_", &blockID);
		ImGui::Text("Enter count:    ");
		ImGui::SameLine();
		if (ImGui::InputInt(".", &count)) { if (count < 0) count = 0; }
		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		wildcard = true;

		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		wildcard = true;
		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		ImGui::Text("Enter amount:        ");
		ImGui::SameLine();
		ImGui::InputInt("..", &amount);
		ImGui::Text("Enter public wallet: ");
		ImGui::SameLine();
		ImGui::InputText("_.", &key);
		break;
	default:
		break;
	}

	ImGui::NewLine();
	displayWidget("Done", [this, wildcard]() {
		if ((key.length() && amount) || (blockID.length() && count || wildcard))
			state = States::INIT_DONE;
		});
}

/*Node setup of IP and port.*/
void GUI::creation() {
	ImGui::Text("Enter IP:   ", ImGuiInputTextFlags_CharsDecimal);
	ImGui::SameLine();

	/*Text input for IP.*/
	ImGui::InputText("", &(nodes.back().ip));
	ImGui::Text("Enter Port: ");
	ImGui::SameLine();

	/*Int input for port.*/
	if (ImGui::InputInt("~  ", &(nodes.back().port)), 1, 5, ImGuiInputTextFlags_CharsDecimal) {
		/*Checks that port>0 or sets it to 0 otherwise.*/
		if (nodes.back().port < 0) nodes.back().port = 0;
	}

	ImGui::NewLine();

	/*'Done' button for finishing setup.*/
	displayWidget("Done", [this]() {if (nodes.back().ip.length()) state = States::INIT; });
}

/*Sets a new ImGUI frame and window.*/
inline void GUI::newWindow() const {
	//Sets new ImGUI frame.
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	//Sets new window positioned at (0,0).
	ImGui::SetNextWindowSize(ImVec2(data::width, data::height));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	/*Begins window and sets title.*/
	ImGui::Begin("EDA - TP7");
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

const std::string& GUI::getBlockID() { return blockID; }
const int GUI::getAmount() { return amount; }
const unsigned int GUI::getCount() { return count; }
const std::string& GUI::getKey() { return key; }

void GUI::infoGotten() {
	key.clear(); amount = 0; count = 0; blockID.clear(); action = Events::NOTHING;
}