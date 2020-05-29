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

	const char* fixedFormat = ".json";
	int notSelectedIndex = -1;
}

const char* messageNone = "none.";
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

bool GUI::nodeSelectionScreen() {
	bool result = false;

	bool endOfSetup = false;
	al_set_target_backbuffer(guiDisp);
	if (state < States::INIT_DONE) {
		while (!endOfSetup) {
			if (eventManager()) { endOfSetup = true; }

			else {
				newWindow();
				ImGui::Text("Initial Setup: ");
				ImGui::NewLine();
				displayWidget("New Node", [this]() { state = States::NODE_SELECTION; });

				if (state == States::NODE_SELECTION) { newNode(); }
				else if (state == States::NODE_CONNECTION) { connections(); }
				else if (state == States::NODE_CREATION) { creation(); }
				else {
					ImGui::NewLine(); ImGui::NewLine();
					displayWidget("Exit", [&endOfSetup]() {endOfSetup = true; });
					ImGui::SameLine();
					displayWidget("Go", [&endOfSetup, this, &result]() {
						if (nodes.size()) { endOfSetup = true; result = true; state = States::INIT_DONE; }});
				}
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

	else if (state >= States::INIT_DONE) {
		/*Sets new ImGui window.*/
		newWindow();

		ImGui::Text("Actions: ");

		ImGui::NewLine();
		displayWidget("New message", [this] {state = States::SENDER_SELECTION; });

		ImGui::NewLine(); ImGui::NewLine();

		if (state == States::SENDER_SELECTION) selectSender();
		else if (state == States::RECEIVER_SELECTION) selectReceiver();
		else if (state == States::MESSAGE_SELECTION) { selectMessage(); if ((bool)action) result = action; }
		else {
			ImGui::NewLine();
			displayWidget("Exit", [&result] {result = Events::END; });
		}

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

void GUI::newNode() {
	ImGui::Text("Select type: ");
	ImGui::SameLine();
	displayWidget("SVP", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_SVP, nodes.size())); state = States::NODE_CONNECTION; });
	ImGui::SameLine();
	displayWidget("FULL", [this]() {nodes.push_back(NewNode(NodeTypes::NEW_FULL, nodes.size())); state = States::NODE_CONNECTION; });
	ImGui::SameLine();
	displayWidget("Delete Last", [this]() {if (nodes.size()) nodes.pop_back(); state = States::INIT; });
}

void GUI::connections() {
	auto nodeValidation = [this](unsigned int index) {
		for (const auto& node : nodes.back().neighbors)
			if (node->index == index)
				return false;
		return true;
	};

	ImGui::NewLine();
	ImGui::Text("Choose connected nodes: ");
	for (unsigned int i = 0; i < nodes.size() - 1; i++) {
		if (nodes[i].type == NodeTypes::NEW_FULL || nodes.back().type == NodeTypes::NEW_FULL) {
			displayWidget(
				("Node " + std::to_string(i)).c_str(),
				[this, i, &nodeValidation]() {
					if (nodeValidation(nodes[i].index)) {
						nodes.back().neighbors.push_back(&nodes[i]);
						nodes[i].neighbors.push_back(&nodes.back());
					}
				}
			);
			ImGui::SameLine();
		}
	}

	showConnections();
}

void GUI::showConnections() {
	ImGui::NewLine(); ImGui::NewLine();
	ImGui::Text("Connected with: ");
	ImGui::SameLine();
	for (const auto& neighbor : nodes.back().neighbors) {
		ImGui::Text(("Node " + std::to_string(neighbor->index)).c_str());
		ImGui::SameLine();
	}
	ImGui::NewLine(); ImGui::NewLine();
	displayWidget("Done", [this]() {state = States::NODE_CREATION; });
}

void GUI::selectSender() {
	ImGui::Text("Select Sender: ");
	for (const auto& node : nodes) {
		displayWidget(("Node " + std::to_string(node.index)).c_str(),
			[this, &node]() {sender = &node; state = States::RECEIVER_SELECTION; });
		ImGui::SameLine();
	}
}

void GUI::selectReceiver() {
	ImGui::Text("Select Receiver: ");

	for (const auto& neighbor : sender->neighbors) {
		displayWidget(("Node " + std::to_string(neighbor->index)).c_str(),
			[this, &neighbor]() {receiver = neighbor; state = States::MESSAGE_SELECTION; });
		ImGui::SameLine();
	}
}

void GUI::selectMessage() {
	action = Events::NOTHING;
	ImGui::Text("Select Message Type: ");
	ImGui::NewLine();

	auto filter = [this]() {displayWidget("Filter (POST)", [this]() {action = Events::FILTER; state = States::INIT_DONE; }); };
	auto get_blocks = [this]() {displayWidget("Block (GET)", [this]() {action = Events::GET_BLOCKS; state = States::INIT_DONE; }); };
	auto get_headers = [this]() {displayWidget("Headers (GET)", [this]() {action = Events::GET_HEADERS;  state = States::INIT_DONE; }); };
	auto merkleblock = [this]() {displayWidget("Merkleblock (POST)", [this]() {action = Events::MERKLEBLOCK; state = States::INIT_DONE; }); };
	auto post_block = [this]() {displayWidget("Block (POST)", [this]() {action = Events::POST_BLOCK; state = States::INIT_DONE; }); };
	auto transaction = [this]() {displayWidget("Transaction (POST)", [this]() {action = Events::TRANSACTION;  state = States::INIT_DONE; }); };

	if (sender->type == NodeTypes::NEW_FULL) {
		if (receiver->type == NodeTypes::NEW_FULL) {
			get_blocks();  ImGui::SameLine();
			post_block();  ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}
		else { merkleblock(); ImGui::NewLine(); }
	}
	else {
		if (receiver->type == NodeTypes::NEW_FULL) {
			filter(); ImGui::SameLine();
			get_headers(); ImGui::SameLine();
			transaction(); ImGui::NewLine();
		}
	}
}

void GUI::creation() {
	ImGui::Text("Enter IP:   ", ImGuiInputTextFlags_CharsDecimal);
	ImGui::SameLine();
	ImGui::InputText("", &nodes.back().ip);
	ImGui::Text("Enter Port: ");
	ImGui::SameLine();
	if (ImGui::InputInt("~  ", &nodes.back().port), 1, 5, ImGuiInputTextFlags_CharsDecimal) {
		if (nodes.back().port < 0)
			nodes.back().port = 0;
	}

	ImGui::NewLine();
	displayWidget("Done", [this]() {state = States::INIT; });
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

const std::vector<GUI::NewNode>& GUI::getNodes() { return nodes; }