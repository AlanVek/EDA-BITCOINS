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
	const unsigned int width = 920;
	const unsigned int height = 500;

	const char* fixedFormat = ".json";
}
/***************************************/

/*GUI constructor. Clears 'format' string and sets Allegro resources.*/
GUI::GUI(void) :
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Events::NOTHING),
	force(true),
	state(States::INIT),
	action_msg("none."),
	index(0),
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

//Cycle that shows menu (called with every iteration).
const Events GUI::checkStatus(void) {
	Events result = Events::NOTHING;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns Events::END.
	if (eventManager())
		result = Events::END;

	else {
		/*Sets new ImGui window.*/
		newWindow();

		/*Text input for new path.*/
		displayPath();

		if (state > States::INIT) {
			ImGui::NewLine(); ImGui::NewLine();

			/*Files from path.*/
			displayFiles();
			displayWidget("Load", [&result, this]() {result = Events::NEW_FILE; state = States::FILE_OK; });

			if (state >= States::FILE_OK) {
				ImGui::NewLine(); ImGui::NewLine();

				displayBlocks();

				if (state >= States::BLOCK_OK) {
					ImGui::NewLine();

					displayActions();
					ImGui::NewLine();
					ImGui::Text("Result: ");
					ImGui::SameLine();
					ImGui::Text(shower.c_str());
					ImGui::NewLine();
					result = action;
				}
			}

			/*Exit button.*/
			displayWidget("Exit", [&result]() {result = Events::END; });

			ImGui::SameLine();
		}

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::Text("Action to perform: ");

	/*Button callback for both buttons.*/
	const auto button_callback = [this](const Events code, const char* msg) {
		action = code;
		action_msg = msg;
	};

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

	/*Message with selected option.*/
	ImGui::Text(("Selected: " + action_msg).c_str());
}
/*Displays text input for path.*/
inline void GUI::displayPath() {
	ImGui::Text("Path: ");
	ImGui::SameLine();
	ImGui::InputText(" ", &path);

	ImGui::SameLine();
	displayWidget("Go", [this]() {if (Filesystem::exists(path.c_str())) {
		fs.newPath(path); state = States::WAITING;
	}});

	ImGui::SameLine();
	displayWidget("Reset", [this]() {path.clear(); state = States::INIT; });
}

/*Displays path and files/folders in path.*/
void GUI::displayFiles() {
	std::string tempPath = fs.getPath();

	ImGui::Text("Current path: ");
	ImGui::SameLine();

	/*Shows path.*/
	ImGui::TextWrapped(tempPath.c_str());

	ImGui::Text("-----------------------------------");
	/*Loops through every file in files map.*/
	for (const auto& file : updateFiles()) {
		/*If it's a file...*/
		if (Filesystem::isFile((tempPath + '\\' + file).c_str())) {
			bool checker = (file == selected);
			/*Sets a checkbox with its name. Updates file's value in map.*/
			displayWidget(std::bind(ImGui::Checkbox, file.c_str(), (bool*)&checker),
				[this, &checker, &file]() {if ((bool)checker) selected = file; });
		}
	}
	ImGui::Text("-----------------------------------");
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

void GUI::displayBlocks(void) {
	for (unsigned int i = 0; i < chainLength; i++) {
		displayWidget(("Block" + std::to_string(i + 1)).c_str(), [this, i]() {state = States::BLOCK_OK; index = i; });
		ImGui::SameLine();
	}
	ImGui::Text(("Selected: " + std::to_string(index + 1)).c_str());
}

/*Getters.*/
const std::string& GUI::getFilename(void) const { return selected; }
const unsigned int GUI::getBlockIndex(void) const { return index; }

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

/*Binding fs.pathContent with this->force and specified file format.
Helps to determine when to update file info.*/
const std::vector<std::string>& GUI::updateFiles(const char* path) {
	bool shouldForce = force;

	force = false;

	return fs.pathContent(path, shouldForce, { data::fixedFormat });
}

void GUI::setChainLength(unsigned int chainLength) { this->chainLength = chainLength; }
void GUI::setInfoShower(const std::string& shower) { this->shower = shower; }