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
	force(true),
	state(States::INIT),
	action_msg(messageNone),
	index(data::notSelectedIndex),
	chainLength(0)
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
	if (state < States::WAITING) {
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

				ImGui::NewLine(); ImGui::NewLine();
				displayWidget("Exit", [&endOfSetup]() {endOfSetup = true; });
				ImGui::SameLine();
				displayWidget("Go", [&endOfSetup, this, &result]() {
					if (nodes.size()) { endOfSetup = true; result = true; state = States::WAITING; }});

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

	else {
		/*Sets new ImGui window.*/
		newWindow();

		/*If it's not the first run after update...*/
		if (state > States::INIT) {
			ImGui::NewLine(); ImGui::NewLine();

			/*Shows files from path.*/
			if (true) {
				result = Events::NEW_FILE;
				state = States::FILE_OK;
			};

			/*If a file has been loaded...*/
			if (state > States::WAITING) {
				ImGui::NewLine(); ImGui::NewLine();

				/*Shows blocks in BlockChain.*/
				//displayBlocks();

				/*If a block was selected...*/
				if (state > States::FILE_OK) {
					ImGui::NewLine();

					/*Shows actions to perform to a given block.*/
					displayActions();
					ImGui::NewLine(); ImGui::NewLine();

					/*If an action has been selected...*/
					if (index != data::notSelectedIndex) {
						/*Shows result of action applied to block.*/
						ImGui::Text("Result: ");
						ImGui::NewLine();
						ImGui::Text(shower.c_str());
						ImGui::NewLine();
						result = action;
					}
				}
			}
		}
		ImGui::NewLine();

		/*Exit button.*/
		displayWidget("Exit", [&result]() {result = Events::END; });

		ImGui::SameLine();

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

void GUI::actionSolved(void) { action = Events::NOTHING; }

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::Text("Action to perform: ");

	/*Button callback for both buttons.*/
	const auto button_callback = [this](const Events code, const char* msg) {
		action = code;
		action_msg = msg;
	};
	/*Creates buttons for different functionalities.*/
	displayWidget("Block ID", std::bind(button_callback, Events::BLOCKID, "Block ID"));
	ImGui::SameLine();
	displayWidget("Previous ID", std::bind(button_callback, Events::PREVIOUS_BLOCKID, "Previous ID"));
	ImGui::SameLine();
	displayWidget("nTx", std::bind(button_callback, Events::NTX, "Number of transactions"));
	ImGui::SameLine();
	displayWidget("Block Number", std::bind(button_callback, Events::BLOCK_NUMBER, "Block Number"));
	ImGui::SameLine();
	displayWidget("Nonce", std::bind(button_callback, Events::NONCE, "Nonce"));
	ImGui::SameLine();
	displayWidget("Calculate MR", std::bind(button_callback, Events::SEE_MROOT, "Merkle Root calculation"));
	ImGui::SameLine();
	displayWidget("Validate MR", std::bind(button_callback, Events::VALIDATE_MROOT, "Merkle Root validation"));
	ImGui::SameLine();
	displayWidget("Print tree", std::bind(button_callback, Events::PRINT_TREE, "Tree printing"));

	/*Message with selected option.*/
	ImGui::Text(("Selected: " + action_msg).c_str());
}
/*Displays text input for path.*/
inline void GUI::displayPath() {
	ImGui::Text("Path: ");
	ImGui::SameLine();
	ImGui::InputText(" ", &path);

	ImGui::SameLine();

	/*Button to go to the written path (if it exists).*/
	displayWidget("Go", [this]() {if (Filesystem::exists(path.c_str())) { fs.newPath(path); state = States::WAITING; }});

	ImGui::SameLine();

	/*Button to go to the original path.*/
	displayWidget("Reset", [this]() {path.clear(); setAllFalse(States::INIT, true); });
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

/*Getters.*/
const std::string& GUI::getFilename(void) { return selected; }
const unsigned int GUI::getBlockIndex(void) const { return index; }

/*Setters.*/
void GUI::setChainLength(unsigned int chainLength) { this->chainLength = chainLength; }
void GUI::setInfoShower(const std::string& shower) { this->shower = shower; }

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

///*Binding fs.pathContent with this->force and specified file format.
//Helps to determine when to update file info.*/
//const std::vector<std::string>& GUI::updateFiles(const char* path) {
//	bool shouldForce = force;
//
//	force = false;
//
//	return fs.pathContent(path, shouldForce, { data::fixedFormat });
//}

///*Sets all parameters to false or null, except (maybe) the ones given as arguments.*/
//inline void GUI::setAllFalse(const States& revert, bool alsoFile) {
//	action = Events::NOTHING;
//	index = data::notSelectedIndex;
//	state = revert;
//	shower = "";
//	action_msg = messageNone;
//	if (alsoFile)
//		selected = "";
//}