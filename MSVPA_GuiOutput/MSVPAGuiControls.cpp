#include "MSVPAGuiControls.h"

MSVPAGuiControls::MSVPAGuiControls()
{
    MainLayt = new QVBoxLayout();

}

QVBoxLayout*
MSVPAGuiControls::mainLayout()
{
    return MainLayt;
}
