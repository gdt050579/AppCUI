include(example)
create_example(Tetris)

target_sources(Tetris PRIVATE State.hpp)
target_sources(Tetris PRIVATE SplashState.hpp)
target_sources(Tetris PRIVATE SplashState.cpp)
target_sources(Tetris PRIVATE MainMenuState.hpp)
target_sources(Tetris PRIVATE MainMenuState.cpp)
target_sources(Tetris PRIVATE RunningState.hpp)
target_sources(Tetris PRIVATE RunningState.cpp)
target_sources(Tetris PRIVATE PauseState.hpp)
target_sources(Tetris PRIVATE PauseState.cpp)
target_sources(Tetris PRIVATE GameOverState.hpp)
target_sources(Tetris PRIVATE GameOverState.cpp)
target_sources(Tetris PRIVATE StateMachine.hpp)
target_sources(Tetris PRIVATE StateMachine.cpp)
target_sources(Tetris PRIVATE Game.hpp)
target_sources(Tetris PRIVATE Game.cpp)
target_sources(Tetris PRIVATE Piece.hpp)
target_sources(Tetris PRIVATE Piece.cpp)
target_sources(Tetris PRIVATE Board.hpp)
target_sources(Tetris PRIVATE Board.cpp)
