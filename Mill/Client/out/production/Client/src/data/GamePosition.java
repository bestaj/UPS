package data;

import data.enums.Stone;
import javafx.scene.shape.Circle;

import java.util.ArrayList;
import java.util.List;

public class GamePosition {

    private final int CIRCLE_RADIUS = 15;
    private final int STONE_RADIUS = 10;
    private final int POS_RADIUS = 5;

    /** List of all neighbour game positions */
    public List<GamePosition> neighbours = new ArrayList<>();
    private double leftUpperCornerX;
    private double leftUpperCornerY;
    /** Defining a circle shape around this game position */
    private Circle circle;
    /** If on this game position is not any stone */
    private boolean free;
    /** Stone on this game position */
    private Stone stone;
    /** Letter specifying a square in which is located this game position
     *  L = Large, M = Mediun, S = Small
     */
 //   private char square;
    /** Index of the game position */
    private int index;

    public GamePosition(double centerX, double centerY, int index) {
        this.index = index;
        this.leftUpperCornerX = centerX - POS_RADIUS;
        this.leftUpperCornerY = centerY - POS_RADIUS;
        this.free = true;
        stone = Stone.NONE;
        this.circle = new Circle(centerX, centerY, CIRCLE_RADIUS);
    }

    public boolean isFree() {
        return free;
    }

    public Stone getStone() {
        return stone;
    }

    public Circle getCircle() {
        return circle;
    }

    public int getIndex() {
        return index;
    }

    public double getLeftUpperCornerX() {
        return leftUpperCornerX;
    }

    public double getLeftUpperCornerY() {
        return leftUpperCornerY;
    }

    public void setPositionFree() {
        this.free = true;
        this.stone = Stone.NONE;
        this.leftUpperCornerX = circle.getCenterX() - POS_RADIUS;
        this.leftUpperCornerY = circle.getCenterY() - POS_RADIUS;
    }

    public void setStonePosition(Stone stone) {
        this.free = false;
        this.stone = stone;
        this.leftUpperCornerX = circle.getCenterX() - STONE_RADIUS;
        this.leftUpperCornerY = circle.getCenterY() - STONE_RADIUS;
    }
}
