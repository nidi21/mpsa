#include "chat.h"
#include "input.h"
#include "network.h"

bool chat::toggled = false;
std::vector<std::string> chat::messages;

void chat::send_message(std::string message) {
	ChatMessage packet;
	if (message.length() > 199)
		return;

	printf("%d\n", message.length());

	strcpy_s(packet.message, message.c_str());

	network::send_packet(CHAT_MESSAGE, &packet, sizeof(ChatMessage));
}

void chat::draw() {
	

	static std::string message = "";

	ImGui::SetNextWindowPos({10,10});
	ImGui::Begin("chat window", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::BeginChild("##messageChild", { 350,200 }))
	{
		for (auto message : messages) {
			ImGui::TextWrapped(message.c_str());
			ImGui::SetScrollHereY(-1.0f);
		}

		ImGui::EndChild();
	}
	if (chat::toggled) {
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.96f);
		;	if (ImGui::InputTextWithHint("##message", "type your message", &message, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
			send_message(message);
			message = "";
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();
}