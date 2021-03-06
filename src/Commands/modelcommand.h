#ifndef MODELCOMMAND_H
#define MODELCOMMAND_H

#include "command.h"
#include <QString>
#include <typeinfo>
#include "global.h"

template<typename T>
class ModelCommand : public Command
{
public:
    ModelCommand(T* model) :
        Command(inferType<T>()),
        m_model(model)
    {
    }

    T* model() const
    {
        return m_model;
    }

private:
    T* m_model;
};

#endif // MODELCOMMAND_H
