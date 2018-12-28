#pragma once
#include <ckcore/types.hh>
#include <ckcore/progress.hh>

class Progress : public ckcore::Progress
{
private:
    unsigned char last_progress_;

public:
    Progress();

    // ckcore::Progress interface.
    void set_progress(unsigned char progress);
    void set_status(const ckcore::tchar *format,...);
    void notify(MessageType type,const ckcore::tchar *format,...);
    bool cancelled();
};

