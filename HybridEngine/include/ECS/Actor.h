#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"

class device;
class MeshComponent;

class 
Actor : public Entity {
public:
  /**
   * @brief Constructor por defecto.
   */
  Actor() = default;

  /**
   * @brief Constructor que inicializa el actor con un dispositivo.
   * @param device El dispositivo con el cual se inicializa el actor.
   */
  Actor(Device& device);

  /**
   * @brief Destructor virtual.
   */
  virtual
  ~Actor() = default;

  /**
   * @brief Actualiza el actor.
   * @param deltaTime El tiempo transcurrido desde la última actualización.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  void
  update(float deltaTime, DeviceContext& deviceContext) override;

  /**
   * @brief Renderiza el actor.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  void
  render(DeviceContext& deviceContext) override;

  /**
   * @brief Destruye el actor y libera los recursos asociados.
   */
  void
  destroy();

  /**
   * @brief Establece las mallas del actor.
   * @param device El dispositivo con el cual se inicializan las mallas.
   * @param meshes Vector de componentes de malla que se van a establecer.
   */
  void
  setMesh(Device& device, std::vector<MeshComponent> meshes);

  std::string
  getName() { 
    return m_name; 
  }
  
  void
  setName(const std::string & name) { 
    m_name = name; 
  }

private:
  std::vector<MeshComponent> m_meshes;  ///< Vector de componentes de malla.
  std::vector<Texture> m_textures;      ///< Vector de texturas.
  std::vector<Buffer> m_vertexBuffers;  ///< Buffers de vértices.
  std::vector<Buffer> m_indexBuffers;   ///< Buffers de índices.
  CBChangesEveryFrame m_model;          ///< Constante del buffer para cambios en cada frame.
  Buffer m_modelBuffer;                 ///< Buffer del modelo.
  std::string m_name = "Actor";         ///< Nombre del actor.
	bool castShadow = false;              ///< Indica si el actor proyecta sombras.
};