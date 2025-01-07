#include <wx/wx.h>
#include <wx/filepicker.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include "ImagePanel.h"
#include "acet.h"
 
class ACETGui : public wxApp
{
public:
    bool OnInit() override;
    bool OnExceptionInMainLoop();
};
 
wxIMPLEMENT_APP(ACETGui);

class MainFrame : public wxFrame
{
public:
    MainFrame();
 
private:
    ImagePanel* imagePanel;
    wxFilePickerCtrl* savePicker;

    void OnMenuOpenSave(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);
    void OnMenuAbout(wxCommandEvent& event);
    void OnMenuExtract(wxCommandEvent& event);
    void OnMenuInject(wxCommandEvent& event);

    void OnSavePicked(wxFileDirPickerEvent& event);

    wxImage ExtractHelper(wxString filename);
};

bool ACETGui::OnInit()
{
    wxInitAllImageHandlers();
    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}

bool ACETGui::OnExceptionInMainLoop()
{
    wxString error;
    try {
        throw; // Rethrow the current exception.
    } catch (const std::exception& e) {
        error = e.what();
    } catch ( ... ) {
        error = "unknown error.";
    }

    wxLogError("Unexpected exception has occurred!\n %s\n The program will terminate.", error);

    // Exit the main loop and thus terminate the program.
    return false;
}
 
MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "ACET Gui")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN, "&Open Save...\tCtrl-O", "Open AC save file to edit");
    auto menuExtractID = menuFile->Append(wxID_ANY, "&Extract Image As...\tCtrl-E",
        "Save current image")->GetId();
    auto menuInjectID = menuFile->Append(wxID_ANY, "&Inject Image...\tCtrl-I",
        "Inject an image into the current save file")->GetId();
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
 
    SetMenuBar( menuBar );

    Bind(wxEVT_MENU, &MainFrame::OnMenuOpenSave, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExtract, this, menuExtractID);
    Bind(wxEVT_MENU, &MainFrame::OnMenuInject, this, menuInjectID);
    Bind(wxEVT_MENU, &MainFrame::OnMenuAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExit, this, wxID_EXIT);

    CreateStatusBar();

    auto mainPanel = new wxPanel(this);
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    savePicker = new wxFilePickerCtrl(mainPanel, wxID_ANY);
    imagePanel = new ImagePanel(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(128,128));
    // auto injectButton = new wxButton(mainPanel, wxID_ANY, "Inject Image");
    // auto extractButton = new wxButton(mainPanel, wxID_ANY, "Extract Image");

    sizer->Add(savePicker, 0, wxEXPAND | wxALL, 10);
    sizer->Add(imagePanel, 1, wxEXPAND);
    // bottomSizer->AddStretchSpacer(1);
    // bottomSizer->Add(extractButton, 0, wxEXPAND | wxRIGHT, 10);
    // bottomSizer->Add(injectButton, 0, wxEXPAND | wxRIGHT, 10);
    // sizer->Add(bottomSizer, 0, wxEXPAND | wxTOP | wxBOTTOM, 10);

    Bind(wxEVT_FILEPICKER_CHANGED, &MainFrame::OnSavePicked, this, savePicker->GetId());

    mainPanel->SetSizer(sizer);
    imagePanel->SetMinSize(wxSize(384,384));
    sizer->SetSizeHints(this);
    // SetClientSize(384, 440);
}
 
void MainFrame::OnMenuExit(wxCommandEvent& event)
{
    Close(true);
}
 
void MainFrame::OnMenuAbout(wxCommandEvent& event)
{
    wxMessageBox("This a tool for injecting and extracting images from PS2 Armored Core emblem saves.",
                 "About ACET-GUI", wxOK | wxICON_INFORMATION);
}

wxImage MainFrame::ExtractHelper(wxString filename) {
    std::ifstream infile((std::string)filename, std::ios::binary | std::ios::ate);
    if (!infile) {
        wxLogError("Failed to load save file");
        return wxNullImage;
    }

	size_t save_size = infile.tellg();
	std::vector<uint8_t> buffer(save_size, 0);

	infile.seekg(0, infile.beg);
	infile.read((char*)buffer.data(), save_size);
	infile.close();

    auto imageRGBA = ExtractImage(buffer);

    auto emblem = wxImage(128, 128, false);
    emblem.InitAlpha();
    emblem.SetType(wxBITMAP_TYPE_PNG);

    for (int i = 0; i < kNumPixels * 4; i += 4) {
        int pixelNumber = i / 4;
        int y = pixelNumber / 128;
        int x = pixelNumber % 128;
        emblem.SetRGB(x, y, imageRGBA[i], imageRGBA[i+1], imageRGBA[i+2]);
        emblem.SetAlpha(x, y, imageRGBA[i+3]);
    }

    if (!emblem.IsOk()) {
        // wxMessageBox("Failed to extract image", "Error", wxOK | wxICON_ERROR);
        wxLogMessage("failed to load image");
        return emblem;
    }

    return emblem;
}

void MainFrame::OnSavePicked(wxFileDirPickerEvent& event) {
    auto savefile = event.GetPath();
    auto emblem = ExtractHelper(savefile);
    imagePanel->SetImage(emblem);
}

void MainFrame::OnMenuOpenSave(wxCommandEvent& event) {
    // wxFileDialog dialog(this, "Open AC2 Emblem Save File", "", "", "AC2 SAVEFILES (BASLUS-21200E*)|BASLUS-21200E*", 
    //     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    wxFileDialog dialog(this);
    
    if (dialog.ShowModal() == wxID_CANCEL)
        return;

    auto savefile = dialog.GetPath();
    auto emblem = ExtractHelper(savefile);

    savePicker->SetPath(savefile);
    imagePanel->SetImage(emblem);
}

void MainFrame::OnMenuExtract(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Save current image", "", "",
        "PNG image (*.png)|*.png|GIF image (*.gif)|*.gif|BMP image (*.bmp)|*.bmp|JPEG image (*.jpg)|*.jpg",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_CANCEL)
        return;

    wxImage image = imagePanel->Image();
    image.SaveFile(dialog.GetPath());
}

void MainFrame::OnMenuInject(wxCommandEvent& event) {
    if (savePicker->GetPath().IsEmpty()) {
        wxLogError("No savefile selected");
        return;
    }
    
    wxFileDialog dialog(this);
    if (dialog.ShowModal() == wxID_CANCEL)
        return;

    auto image = wxImage(dialog.GetPath());
    if (!image.IsOk()) {
        wxLogError("Image not ok");
        return;
    }

    if (!image.HasAlpha())
        image.InitAlpha();

    auto backup = wxGetCwd() + "/acet-backup";

    if (!wxDirExists(backup) && !wxMkdir(backup)) {
        wxLogError("couldn't make acet-backup");
        return;
    }

    backup += wxString::Format(wxT("/%d"), wxGetLocalTime());
    auto savename = savePicker->GetPath();

    if (!wxDirExists(backup) && !wxMkdir(backup)) {
        wxLogError("couldn't make " + backup);
        return;
    }

    if (!wxCopyFile(savePicker->GetPath(), backup + "/" + savePicker->GetFileName().GetName())) {
        wxLogError("couldn't copy");
        return;
    }

	std::vector<uint8_t> save = ReadSaveFile((std::string)savename);
	std::vector<uint8_t> inImage;

    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++) {
            inImage.push_back(image.GetRed(x, y));
            inImage.push_back(image.GetGreen(x, y));
            inImage.push_back(image.GetBlue(x, y));
            inImage.push_back(image.GetAlpha(x, y));
        }

	InjectImage(save, inImage);

	std::ofstream outfile((std::string)savename, std::ios::binary);
	outfile.write((char*)save.data(), save.size());
	outfile.close();

    auto emblem = ExtractHelper(savename);
    imagePanel->SetImage(emblem);
}